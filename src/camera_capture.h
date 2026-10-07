/*
 * camera_capture.h — public API for the portable camera capture.
 *
 *     static uint8_t frame[CAM_FRAME_BYTES];
 *     camera_capture_init(frame);
 *     for (;;) {
 *         camera_capture_service();
 *         if (camera_capture_frame_ready()) {
 *             // consume RGB565 pixels, 160 x 120 little-endian
 *         }
 *     }
 */

#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bring up EIC/EVSYS, MCLK, sensor, and prime the DMA. `frame_buf` must be
 * at least CAM_FRAME_BYTES and stay valid for the lifetime of the capture. */
void camera_capture_init(uint8_t *frame_buf);

void camera_capture_start(void);
void camera_capture_stop(void);
void camera_capture_pause(void);    /* freeze DMA mid-stream, hold buffer */
void camera_capture_unpause(void);  /* resume from line 0 of next frame   */

/* Re-arm after stop() without touching sensor or clocks. */
void camera_capture_resume(void);

bool camera_capture_frame_ready(void);
const uint8_t *camera_capture_get_frame(void);
void camera_capture_release_frame(void);

/* Call from the main loop. Promotes DMA-done frames to consumer-visible
 * after any required cache maintenance. */
void camera_capture_service(void);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_CAPTURE_H */
