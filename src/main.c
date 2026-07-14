/*
 * Continuous OV7670 capture; SW0 dumps the most recent frame over UART.
 *
 * Frame wire format (SERCOM5, 460800 8N1):
 *   [FF 00 FE 01] [W_lo W_hi] [H_lo H_hi]  <W*H*2 bytes RGB565 LE>  [AA 55 AA 55]
 *     magic         width       height           payload             tail
 *
 * The buffer is single-buffered, so capture is paused for the ~0.83 s
 * duration of the UART transfer. See tools/receive_frame.py to decode.
 *
 * SERCOM2 USART: PB24=TX PAD0, PB25=RX PAD1, 460800 8N1 (stdio)
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "definitions.h"
#include "camera_capture.h"
#include "camera_config.h"
#include "camera_hal.h"

/* SW0 = PB31, active low, pull-up. */
static inline bool sw0_pressed(void) { return SW0_Get() == 0U; }

static uint8_t frame[CAM_FRAME_BYTES];

/* Freeze/resume gate the continuous-capture loop from camera_capture.c by
 * masking the VSYNC EIC line and stopping the DMA. */
static void capture_freeze(void)
{
    EIC_InterruptDisable((EIC_PIN)CAM_EIC_VSYNC_LINE);
    camera_capture_stop();
}

static void capture_resume(void)
{
    camera_capture_resume();
    EIC_InterruptEnable((EIC_PIN)CAM_EIC_VSYNC_LINE);
}

/* Wait for a fresh post-press frame to complete, then freeze so the buffer
 * is stable for the UART dump. */
static void freeze_on_latest_frame(void)
{
    if (camera_capture_frame_ready()) {
        camera_capture_release_frame();
    }
    while (!camera_capture_frame_ready()) {
        camera_capture_service();
    }
    capture_freeze();
    camera_capture_release_frame();
}

static void uart_dump_frame(const uint8_t *buf, uint32_t len)
{
    uint8_t hdr[8] = {
        0xFFU, 0x00U, 0xFEU, 0x01U,
        (uint8_t)(CAM_FRAME_WIDTH  & 0xFFU),
        (uint8_t)((CAM_FRAME_WIDTH  >> 8) & 0xFFU),
        (uint8_t)(CAM_FRAME_HEIGHT & 0xFFU),
        (uint8_t)((CAM_FRAME_HEIGHT >> 8) & 0xFFU),
    };
    uint8_t tail[4] = { 0xAAU, 0x55U, 0xAAU, 0x55U };

    (void)SERCOM2_USART_Write(hdr, sizeof hdr);
    (void)SERCOM2_USART_Write((void *)(uintptr_t)buf, len);
    (void)SERCOM2_USART_Write(tail, sizeof tail);
}

int main(void)
{
    SYS_Initialize(NULL);
    camera_capture_init(frame);

    bool press_latched = false;

    while (true)
    {
        camera_capture_service();

        if (!press_latched && sw0_pressed()) {
            cam_hal_delay_ms(20);            /* debounce */
            if (sw0_pressed()) {
                press_latched = true;
                freeze_on_latest_frame();
                uart_dump_frame(frame, CAM_FRAME_BYTES);
                capture_resume();
            }
        }

        if (press_latched && !sw0_pressed()) {
            cam_hal_delay_ms(20);
            if (!sw0_pressed()) {
                press_latched = false;
            }
        }
    }

    return EXIT_FAILURE;
}
