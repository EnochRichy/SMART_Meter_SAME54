/*******************************************************************************
  Application Header — USB Vendor bulk camera stream
 ******************************************************************************/

#ifndef _APP_H
#define _APP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Set to 0 to disable USB camera streaming (inference-only mode) */
#define USB_STREAM_ENABLED  0

#define APP_EP_BULK_IN  (1U | USB_EP_DIRECTION_IN)
#define APP_EP_BULK_OUT (1U | USB_EP_DIRECTION_OUT)

typedef enum
{
    APP_STATE_INIT = 0,
    APP_STATE_WAIT_FOR_CONFIGURATION,
    APP_STATE_MAIN_TASK,
    APP_STATE_ERROR,
} APP_STATES;

typedef struct
{
    APP_STATES              state;
    USB_DEVICE_HANDLE       deviceHandle;
    bool                    deviceIsConfigured;
    bool                    epDataWritePending;
    USB_DEVICE_TRANSFER_HANDLE writeTransferHandle;
    USB_DEVICE_TRANSFER_HANDLE readTransferHandle;
    uint16_t                endpointMaxPktSize;
    uint8_t                 altSetting;
} APP_DATA;

extern APP_DATA appData;

void APP_Initialize(void);
void APP_Tasks(void);
void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void *eventData, uintptr_t context);

#ifdef __cplusplus
}
#endif

#endif /* _APP_H */
