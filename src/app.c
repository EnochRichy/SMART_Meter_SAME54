/*******************************************************************************
  USB Vendor bulk camera stream — modelled on PersonDetection/app_usb.c

  Wire format per frame (38 408 bytes total):
    [0..3]   AA 55 AA 55  — sync marker
    [4..5]   width  (u16 LE)
    [6..7]   height (u16 LE)
    [8..38407] 160*120*2 bytes of RGB565 payload (little-endian)

  Transfer sequence (task-context driven, NOT from callback):
    1. APP_Tasks detects frame ready → writes 8-byte header with
       MORE_DATA_PENDING, sets txPacketIndex = 0.
    2. WRITE_COMPLETE sets epDataWritePending = false.
    3. APP_Tasks detects epDataWritePending==false while txPacketIndex < NUM_PACKETS
       → queues next 64-byte payload chunk.
    4. After last chunk (DATA_COMPLETE), APP_Tasks releases the frame.
 ******************************************************************************/

#include "app.h"

#if !USB_STREAM_ENABLED

APP_DATA appData;
void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void *eventData,
                               uintptr_t context) { (void)event; (void)eventData; (void)context; }
void APP_Initialize(void) { appData.state = APP_STATE_INIT; }
void APP_Tasks(void) {}

#else /* USB_STREAM_ENABLED */

#include "camera_capture.h"
#include "camera_config.h"
#include "app_ml.h"
#include <string.h>
#include <stdio.h>

static uint32_t s_dbgLoopCount = 0;
static uint32_t s_dbgFrameCount = 0;

#define FRAME_PACKET_SIZE   64U
#define FRAME_PAYLOAD_SIZE  CAM_FRAME_BYTES                             /* 38400 */
#define FRAME_HEADER_SIZE   16U
#define FRAME_NUM_PACKETS   (FRAME_PAYLOAD_SIZE / FRAME_PACKET_SIZE)    /* 600  */

static uint8_t         s_header[FRAME_HEADER_SIZE];
static uint32_t        s_txPacketIndex = 0;
static const uint8_t  *s_frame = NULL;
/* Phase: 0 = send header, 1 = send payload packets */
static uint8_t         s_txPhase = 0;
/* Set after the last packet is submitted; frame released only once write completes */
static bool            s_releasePending = false;

APP_DATA appData;

/*******************************************************************************
 * USB Device Event Handler — only sets flags, never calls EndpointWrite
 ******************************************************************************/
void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void *eventData,
                               uintptr_t context)
{
    uint8_t *configValue;
    USB_SETUP_PACKET *setup;

    switch (event)
    {
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_DECONFIGURED:
            appData.deviceIsConfigured = false;
            break;

        case USB_DEVICE_EVENT_CONFIGURED:
            configValue = (uint8_t *)eventData;
            if (*configValue == 1U)
            {
                appData.deviceIsConfigured = true;
            }
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:
            USB_DEVICE_Attach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:
            USB_DEVICE_Detach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
            setup = (USB_SETUP_PACKET *)eventData;
            if (setup->bRequest == USB_REQUEST_SET_INTERFACE)
            {
                USB_DEVICE_ControlStatus(appData.deviceHandle,
                                         USB_DEVICE_CONTROL_STATUS_OK);
            }
            else if (setup->bRequest == USB_REQUEST_GET_INTERFACE)
            {
                USB_DEVICE_ControlSend(appData.deviceHandle,
                                       &appData.altSetting, 1);
            }
            else
            {
                USB_DEVICE_ControlStatus(appData.deviceHandle,
                                         USB_DEVICE_CONTROL_STATUS_ERROR);
            }
            break;

        case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
            appData.epDataWritePending = false;
            if (s_releasePending)
            {
                s_releasePending = false;
                camera_capture_release_frame();
                s_frame   = NULL;
                s_txPhase = 0U;
                camera_capture_unpause();
                printf("[APP] frame %lu: TX done, released\r\n", s_dbgFrameCount);
            }
            break;

        case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
            appData.epDataWritePending = false;
            break;

        case USB_DEVICE_EVENT_SUSPENDED:
        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
            break;
    }
}

/*******************************************************************************
 * APP_Initialize
 ******************************************************************************/
void APP_Initialize(void)
{
    appData.state               = APP_STATE_INIT;
    appData.deviceHandle        = USB_DEVICE_HANDLE_INVALID;
    appData.deviceIsConfigured  = false;
    appData.epDataWritePending  = false;
    appData.writeTransferHandle = USB_DEVICE_TRANSFER_HANDLE_INVALID;
    appData.readTransferHandle  = USB_DEVICE_TRANSFER_HANDLE_INVALID;
    appData.endpointMaxPktSize  = 64U;
    appData.altSetting          = 0U;

    s_txPacketIndex  = 0U;
    s_txPhase        = 0U;
    s_frame          = NULL;
    s_releasePending = false;
    memset(s_header, 0, sizeof(s_header));
}

/*******************************************************************************
 * APP_Tasks — called from SYS_Tasks every main-loop iteration
 ******************************************************************************/
void APP_Tasks(void)
{
    switch (appData.state)
    {
        case APP_STATE_INIT:
            appData.deviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0,
                                                   DRV_IO_INTENT_READWRITE);
            if (appData.deviceHandle != USB_DEVICE_HANDLE_INVALID)
            {
                USB_DEVICE_EventHandlerSet(appData.deviceHandle,
                                           APP_USBDeviceEventHandler, 0U);
                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
                printf("[APP] USB opened, waiting for host config\r\n");
            }
            break;

        case APP_STATE_WAIT_FOR_CONFIGURATION:
            if (appData.deviceIsConfigured)
            {
                appData.endpointMaxPktSize = 64U;

                if (!USB_DEVICE_EndpointIsEnabled(appData.deviceHandle,
                                                  APP_EP_BULK_OUT))
                {
                    USB_DEVICE_EndpointEnable(appData.deviceHandle, 0U,
                        APP_EP_BULK_OUT, USB_TRANSFER_TYPE_BULK,
                        appData.endpointMaxPktSize);
                }
                if (!USB_DEVICE_EndpointIsEnabled(appData.deviceHandle,
                                                  APP_EP_BULK_IN))
                {
                    USB_DEVICE_EndpointEnable(appData.deviceHandle, 0U,
                        APP_EP_BULK_IN, USB_TRANSFER_TYPE_BULK,
                        appData.endpointMaxPktSize);
                }

                appData.state = APP_STATE_MAIN_TASK;
                printf("[APP] USB configured, endpoints enabled\r\n");
            }
            break;

        case APP_STATE_MAIN_TASK:
            if (!appData.deviceIsConfigured)
            {
                USB_DEVICE_EndpointDisable(appData.deviceHandle, APP_EP_BULK_OUT);
                USB_DEVICE_EndpointDisable(appData.deviceHandle, APP_EP_BULK_IN);
                appData.epDataWritePending = false;
                s_releasePending = false;
                if (s_frame != NULL)
                {
                    camera_capture_release_frame();
                    s_frame = NULL;
                }
                s_txPhase       = 0U;
                s_txPacketIndex = 0U;
                camera_capture_unpause();
                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
                break;
            }

            /* Periodic heartbeat so we know the loop is alive */
            if ((s_dbgLoopCount++ & 0xFFFFFU) == 0U)
            {
                printf("[APP] loop=%lu frames=%lu ready=%d pending=%d phase=%u\r\n",
                       s_dbgLoopCount, s_dbgFrameCount,
                       (int)camera_capture_frame_ready(),
                       (int)appData.epDataWritePending,
                       (unsigned)s_txPhase);
            }

            /* Nothing in flight — decide what to send next */
            if (!appData.epDataWritePending)
            {
                if (s_txPhase == 0U && camera_capture_frame_ready())
                {
                    /* --- Start a new frame: send 8-byte header --- */
                    s_frame = camera_capture_get_frame();
                    s_dbgFrameCount++;

                    s_header[0] = 0xAAU;
                    s_header[1] = 0x55U;
                    s_header[2] = 0xAAU;
                    s_header[3] = 0x55U;
                    {
                        uint32_t cnt = s_dbgFrameCount;
                        s_header[4] = (uint8_t)(cnt & 0xFFU);
                        s_header[5] = (uint8_t)((cnt >> 8) & 0xFFU);
                        s_header[6] = (uint8_t)((cnt >> 16) & 0xFFU);
                        s_header[7] = (uint8_t)((cnt >> 24) & 0xFFU);
                    }
                    s_header[8]  = APP_ML_GetPersonPresent() ? 1U : 0U;
                    s_header[9]  = (uint8_t)APP_ML_GetPersonLogit();
                    s_header[10] = (uint8_t)APP_ML_GetNoPersonLogit();
                    s_header[11] = APP_ML_GetInferenceCount();
                    {
                        uint32_t inf_us = APP_ML_GetInferenceUs();
                        s_header[12] = (uint8_t)(inf_us & 0xFFU);
                        s_header[13] = (uint8_t)((inf_us >> 8) & 0xFFU);
                        s_header[14] = (uint8_t)((inf_us >> 16) & 0xFFU);
                        s_header[15] = (uint8_t)((inf_us >> 24) & 0xFFU);
                    }

                    s_txPacketIndex            = 0U;
                    s_txPhase                  = 1U;
                    appData.epDataWritePending = true;
                    camera_capture_pause();

                    printf("[APP] frame %lu: sending header\r\n", s_dbgFrameCount);

                    USB_DEVICE_EndpointWrite(
                        appData.deviceHandle,
                        &appData.writeTransferHandle,
                        APP_EP_BULK_IN,
                        s_header,
                        FRAME_HEADER_SIZE,
                        USB_DEVICE_TRANSFER_FLAGS_MORE_DATA_PENDING);
                }
                else if (s_txPhase == 1U && s_frame != NULL)
                {
                    /* --- Send next 64-byte payload chunk --- */
                    uint32_t offset = s_txPacketIndex * FRAME_PACKET_SIZE;
                    bool isLast = (s_txPacketIndex == FRAME_NUM_PACKETS - 1U);

                    appData.epDataWritePending = true;

                    USB_DEVICE_EndpointWrite(
                        appData.deviceHandle,
                        &appData.writeTransferHandle,
                        APP_EP_BULK_IN,
                        (const void *)(s_frame + offset),
                        FRAME_PACKET_SIZE,
                        isLast ? USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE
                               : USB_DEVICE_TRANSFER_FLAGS_MORE_DATA_PENDING);

                    s_txPacketIndex++;

                    if (isLast)
                    {
                        /* Don't release yet — buffer is still in flight.
                         * Callback clears s_releasePending once USB confirms done. */
                        s_releasePending = true;
                    }
                }
            }
            break;

        case APP_STATE_ERROR:
        default:
            break;
    }
}

#endif /* USB_STREAM_ENABLED */
