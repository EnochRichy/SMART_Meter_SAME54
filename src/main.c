/*
 * Continuous OV7670 capture streamed over USB CDC.
 *
 * Frame wire format:
 *   [FF 00 FE 01] [W_lo W_hi] [H_lo H_hi]  <W*H*2 bytes RGB565 LE>  [AA 55 AA 55]
 *
 * USB application logic lives in app.c / app.h.
 * Use tools/receive_frame.py on the host to decode.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "definitions.h"
#include "camera_capture.h"
#include "camera_config.h"
#include "app_ml.h"

static uint8_t frame[CAM_FRAME_BYTES];

int main(void)
{
    SYS_Initialize(NULL);
    camera_capture_init(frame);
    APP_ML_Initialize();

    printf("---- PersonPresence E54 ----\r\n");
    while (true)
    {
        camera_capture_service();
        APP_ML_Tasks();
        SYS_Tasks();
    }

    return EXIT_FAILURE;
}
