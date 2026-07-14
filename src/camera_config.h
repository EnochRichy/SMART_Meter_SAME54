/*
 * camera_config.h — target-specific knobs for the portable camera capture
 *
 * This is the ONE file you should have to touch when porting to a new MCU
 * (SAM E54, SAM D5x, PIC32CX, etc.). It holds:
 *   - Frame geometry (must match ov7670 register-init)
 *   - Which GPIO PORT byte-lane the 8 camera data lines land on
 *   - Which DMA channel, EVSYS channel, EVSYS user, EIC lines to use
 *   - Where "end of frame" is detected in the VSYNC handler
 *
 * The values below are the reference wiring from the PIC32CZ8110CA90208
 * PersonPresence project. Replace them for your target.
 */

#ifndef CAMERA_CONFIG_H
#define CAMERA_CONFIG_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Frame geometry (OV7670 QQVGA RGB565)
 * ------------------------------------------------------------------------- */
#define CAM_FRAME_WIDTH             160U
#define CAM_FRAME_HEIGHT            120U
#define CAM_BYTES_PER_PIXEL         2U
#define CAM_FRAME_BYTES             (CAM_FRAME_WIDTH * CAM_FRAME_HEIGHT * CAM_BYTES_PER_PIXEL)

/* End-of-frame detection: VSYNC ISR rearms the DMA when line_index reaches
 * this many lines. Slightly less than CAM_FRAME_HEIGHT gives the DMA a
 * conservative margin — the last few HREFs after the sensor's active region
 * are ignored so the DMA is already re-primed by the next VSYNC. */
#define CAM_END_OF_FRAME_LINE_THRESHOLD  (CAM_FRAME_HEIGHT - 5U)

/* ---------------------------------------------------------------------------
 * Data-bus wiring
 *
 * The 8 camera data lines D0..D7 MUST land on 8 consecutive bits of a single
 * GPIO PORT group so one byte-lane of that group's IN register holds all 8
 * bits in one read. CAM_PORT_IN_BYTE_ADDR points at that byte-lane and is
 * fed to the DMA as a FIXED source address (source-address step size = 0).
 *
 * Reference (PIC32CZ): D0..D7 on PC08..PC15 -> high byte of PORTC IN.
 * SAM E54: choose 8 consecutive PORT_A/B/C/D pins; the byte-lane expression
 * is (uint8_t *)&PORT_REGS->Group[G].IN.reg + (bit_offset / 8).
 * ------------------------------------------------------------------------- */
/* Defined at compile time by the port shim (camera_hal_<target>.c) because
 * it needs the target's SFR headers. See camera_hal.h::cam_hal_port_in_byte(). */

/* ---------------------------------------------------------------------------
 * DMA / EVSYS / EIC assignments
 *
 * These are IDs, not physical pins. The port shim is responsible for
 * programming the peripheral registers with these values.
 *
 * SAM E54 (ATSAME54P20A 144-pin LQFP) wiring:
 *   - DMA channel 0  drives the byte capture
 *   - EVSYS channel 0 carries PCLK events
 *   - EVSYS user  5  = EVENT_ID_USER_DMAC_CH_0 (DMAC channel-0 trigger user)
 *   - EIC EXTINT  0  = PCLK  on PB16 (falling, event-only, no CPU IRQ)
 *   - EIC EXTINT 14  = HSYNC on PB14 (rising, CPU IRQ)
 *   - EIC EXTINT 15  = VSYNC on PB15 (rising, CPU IRQ)
 * ------------------------------------------------------------------------- */
#define CAM_DMA_CHANNEL             0U
#define CAM_EVSYS_CHANNEL           0U
#define CAM_EVSYS_USER_DMAC_TRIGGER 5U

#define CAM_EIC_PCLK_LINE           0U
#define CAM_EIC_HSYNC_LINE          14U
#define CAM_EIC_VSYNC_LINE          15U

/* ---------------------------------------------------------------------------
 * OV7670 I2C address (SCCB)
 * ------------------------------------------------------------------------- */
#define OV7670_I2C_ADDR_7BIT        0x21U   /* 0x42 >> 1 */
#define OV7670_I2C_ADDR_READ_7BIT   0x43U   /* 0x86 >> 1 */

#endif /* CAMERA_CONFIG_H */
