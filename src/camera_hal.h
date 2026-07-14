/*
 * camera_hal.h — platform abstraction for the portable camera capture.
 *
 * To port to a new MCU: write a camera_hal_<target>.c implementing every
 * prototype below, edit camera_config.h for the target's DMA/EVSYS/EIC IDs,
 * and route the 8 camera data pins to 8 consecutive bits of one PORT group
 * (cam_hal_port_in_byte() returns that byte-lane).
 */

#ifndef CAMERA_HAL_H
#define CAMERA_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*cam_hal_isr_cb_t)(void);
typedef void (*cam_hal_dma_done_cb_t)(void);

bool cam_hal_i2c_write(uint8_t reg_addr, uint8_t reg_value);

void cam_hal_reset_assert(void);
void cam_hal_reset_deassert(void);

void cam_hal_delay_ms(uint32_t ms);

/* MCLK must be running before any I2C to the sensor — OV7670's state
 * machine is clocked by MCLK. */
void cam_hal_mclk_start(void);
void cam_hal_mclk_stop(void);

/* Byte-lane of a GPIO PORT IN register carrying the 8 camera data bits.
 * DMA reads it as a fixed source (no auto-increment) per PCLK event. */
const volatile uint8_t *cam_hal_port_in_byte(void);

/* Configure the capture DMA channel:
 *   src = cam_hal_port_in_byte() (no increment)
 *   dst = dest_buf (byte increment)
 *   beat = 1 byte, block = len, trigger = one beat per EVSYS event,
 *   block-complete IRQ = enabled. */
void cam_hal_dma_configure_byte_capture(uint8_t *dest_buf, uint32_t len);
void cam_hal_dma_start(void);
void cam_hal_dma_stop(void);
void cam_hal_dma_register_block_complete_cb(cam_hal_dma_done_cb_t cb);

/* Event fabric wiring:
 *   EIC PCLK line  : falling sense, event-output on, CPU IRQ off.
 *   EIC HSYNC line : rising sense, CPU IRQ on, glitch filter on.
 *   EIC VSYNC line : rising sense, CPU IRQ on, glitch filter on.
 *   EVSYS channel  : gen = PCLK EIC line, user = DMAC channel trigger. */
void cam_hal_eic_register_hsync_cb(cam_hal_isr_cb_t cb);
void cam_hal_eic_register_vsync_cb(cam_hal_isr_cb_t cb);

/* D-cache maintenance after DMA writes into a CPU-readable buffer. On
 * cores with a D-cache the correct op is INVALIDATE; no-op on Cortex-M4. */
void cam_hal_dcache_sync_after_capture(void *addr, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_HAL_H */
