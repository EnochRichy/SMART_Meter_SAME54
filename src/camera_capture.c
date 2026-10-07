/*
 * camera_capture.c — DMA + EIC/EVSYS state machine for OV7670 parallel capture
 *
 *   - PCLK drives an EVSYS channel; each edge triggers one DMA byte transfer
 *     from PORT->IN into the frame buffer.
 *   - Each HSYNC rising re-arms the DMA at frame_buf[N * CAM_LINE_BYTES] for
 *     exactly one line. This gives per-line alignment even if HBLANK PCLK
 *     leaks between lines: stray events either land in the already-full
 *     previous descriptor (dropped) or fire during the ~500 ns reconfigure
 *     window (channel disabled, also dropped).
 *   - VSYNC rising signals frame-ready if enough HREFs were counted; it
 *     does not rearm the DMA — the next frame's first HSYNC does.
 *
 * All peripheral access is behind camera_hal.h; this file is portable
 * across MCUs with EIC + EVSYS + DMAC.
 */

#include "camera_capture.h"
#include "camera_config.h"
#include "camera_hal.h"
#include "ov7670.h"
#include <stdio.h>

static volatile uint32_t s_dbg_hsync_count = 0;
static volatile uint32_t s_dbg_vsync_count = 0;
static volatile uint32_t s_dbg_dma_done_count = 0;
static uint32_t          s_dbg_svc_calls = 0;

/* Two flags on purpose: s_dma_done is raised in ISR context and consumed by
 * service(); s_frame_visible is raised by service() only after cache
 * maintenance so the consumer never sees a half-flushed buffer. */
static uint8_t          *s_frame_buf     = 0;
static volatile bool     s_dma_done      = false;
static volatile bool     s_frame_visible = false;
static volatile uint32_t s_line_index    = 0;
static volatile bool     s_paused        = false;

#define CAM_LINE_BYTES  ((uint32_t)(CAM_FRAME_WIDTH * 2U))

static void on_dma_block_complete(void)
{
    /* Fires per-line under the per-HSYNC-rearm design; frame-ready is
     * raised from on_vsync instead. Intentionally a no-op. */
}

static void on_hsync(void)
{
    s_dbg_hsync_count++;
    if (s_paused)
        return;
    if (s_line_index < CAM_FRAME_HEIGHT) {
        cam_hal_dma_stop();
        cam_hal_dma_configure_byte_capture(
            s_frame_buf + (s_line_index * CAM_LINE_BYTES),
            CAM_LINE_BYTES);
    }
    s_line_index++;
}

static void on_vsync(void)
{
    s_dbg_vsync_count++;
    if (s_line_index >= CAM_END_OF_FRAME_LINE_THRESHOLD) {
        cam_hal_dma_stop();
        s_dma_done = true;
        s_dbg_dma_done_count++;
    }
    s_line_index = 0;
}

void camera_capture_init(uint8_t *frame_buf)
{
    s_frame_buf     = frame_buf;
    s_dma_done      = false;
    s_frame_visible = false;
    s_line_index    = 0;

    cam_hal_eic_register_hsync_cb(on_hsync);
    cam_hal_eic_register_vsync_cb(on_vsync);
    cam_hal_dma_register_block_complete_cb(on_dma_block_complete);

    /* MCLK must be running before I2C — the sensor's state machine is
     * clocked by MCLK. */
    cam_hal_mclk_start();
    ov7670_init_rgb565_qqvga();

    cam_hal_dma_configure_byte_capture(s_frame_buf, CAM_LINE_BYTES);
    camera_capture_start();
}

void camera_capture_start(void)
{
    cam_hal_dma_start();
}

void camera_capture_stop(void)
{
    cam_hal_dma_stop();
}

void camera_capture_pause(void)
{
    s_paused = true;
    cam_hal_dma_stop();
}

void camera_capture_unpause(void)
{
    s_paused        = false;
    s_line_index    = 0;
    s_dma_done      = false;
    cam_hal_dma_configure_byte_capture(s_frame_buf, CAM_LINE_BYTES);
}

void camera_capture_resume(void)
{
    s_line_index    = 0;
    s_dma_done      = false;
    s_frame_visible = false;
    cam_hal_dma_configure_byte_capture(s_frame_buf, CAM_LINE_BYTES);
    cam_hal_dma_start();
}

bool camera_capture_frame_ready(void)
{
    return s_frame_visible;
}

const uint8_t *camera_capture_get_frame(void)
{
    return s_frame_buf;
}

void camera_capture_release_frame(void)
{
    s_frame_visible = false;
}

void camera_capture_service(void)
{
    if ((s_dbg_svc_calls++ & 0xFFFFFU) == 0U)
    {
        printf("[CAM] hsync=%lu vsync=%lu done=%lu lines=%lu paused=%d\r\n",
               s_dbg_hsync_count, s_dbg_vsync_count,
               s_dbg_dma_done_count, s_line_index, (int)s_paused);
    }

    if (s_dma_done && !s_frame_visible) {
        s_dma_done = false;
        cam_hal_dcache_sync_after_capture(s_frame_buf, CAM_FRAME_BYTES);
        s_frame_visible = true;
    }
}
