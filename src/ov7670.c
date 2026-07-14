/*
 * ov7670.c — OV7670 register-init for RGB565 QQVGA.
 *
 * Register values are drawn from the Linux ov7670 driver, with a few
 * empirically-tuned tweaks. Only the surprising / load-bearing settings
 * are commented; ordinary datasheet defaults are not.
 */

#include "ov7670.h"
#include "camera_hal.h"
#include "camera_config.h"

static void short_reset_settle(void) { cam_hal_delay_ms(5); }

void ov7670_reset(void)
{
    cam_hal_reset_assert();
    short_reset_settle();
    cam_hal_reset_deassert();
    short_reset_settle();

    cam_hal_i2c_write(OV7670_REG_COM7, OV7670_COM7_RESET);
    cam_hal_delay_ms(10);
}

void ov7670_init_rgb565_qqvga(void)
{
    ov7670_reset();

    cam_hal_i2c_write(OV7670_REG_GAIN, 0x0B);
    cam_hal_i2c_write(OV7670_REG_CLKRC, 0x80);              /* prescaler /1 */
    cam_hal_i2c_write(OV7670_REG_COM7,  OV7670_COM7_RGB_QVGA);

    /* COM8: enable Fast AEC + AEC + AGC + AWB. Leaving AGC or AWB off
     * locks the sensor into a fixed cast tied to the room illuminant. */
    cam_hal_i2c_write(OV7670_REG_COM8, 0x87);

    /* AGC ceiling capped at 2x to keep highlights from blowing out
     * once the color matrix passes full red. */
    cam_hal_i2c_write(OV7670_REG_COM9, 0x18);
    cam_hal_i2c_write(OV7670_REG_AEW,  0x70);
    cam_hal_i2c_write(OV7670_REG_AEB,  0x60);
    cam_hal_i2c_write(OV7670_REG_VPT,  0xA5);

    cam_hal_i2c_write(OV7670_REG_RGB444, 0x00);
    cam_hal_i2c_write(OV7670_REG_TSLB,   0x08);             /* byte-swap for LE decode */
    cam_hal_i2c_write(OV7670_REG_COM13,  0x88);
    cam_hal_i2c_write(OV7670_REG_PCLK_DELAY, 0x02);
    cam_hal_i2c_write(OV7670_REG_COM15,  0xD0);             /* full-range RGB565 */
    cam_hal_i2c_write(OV7670_REG_COM10,  0x20);             /* gate PCLK during HBLANK */

    cam_hal_i2c_write(OV7670_REG_COM3,   0x0C);             /* DCW + Scale enable */
    cam_hal_i2c_write(OV7670_REG_COM14,  0x12);
    cam_hal_i2c_write(OV7670_REG_COM5,   0x01);

    cam_hal_i2c_write(OV7670_REG_SCALING_XSC,  0x3F);
    cam_hal_i2c_write(OV7670_REG_SCALING_YSC,  0x35);
    cam_hal_i2c_write(OV7670_REG_SCALING_DCW,  0x21);
    cam_hal_i2c_write(OV7670_REG_SCALING_PCLK, 0xF0);

    cam_hal_i2c_write(OV7670_REG_HSTART, 0x16);
    cam_hal_i2c_write(OV7670_REG_HSTOP,  0x04);
    cam_hal_i2c_write(OV7670_REG_HREF,   0xA4);
    cam_hal_i2c_write(OV7670_REG_VSTART, 0x02);
    cam_hal_i2c_write(OV7670_REG_VSTOP,  0x7A);
    cam_hal_i2c_write(OV7670_REG_VREF,   0x0A);

    /* Color matrix — all 6 coefficients plus MTXS (per-coefficient signs)
     * are required. Writing only MTX1..MTX3 leaves the YUV->RGB basis
     * incomplete and produces a strong cyan cast. */
    cam_hal_i2c_write(OV7670_REG_MTX1, 0x80);
    cam_hal_i2c_write(OV7670_REG_MTX2, 0x80);
    cam_hal_i2c_write(OV7670_REG_MTX3, 0x00);
    cam_hal_i2c_write(OV7670_REG_MTX4, 0x22);
    cam_hal_i2c_write(OV7670_REG_MTX5, 0x5E);
    cam_hal_i2c_write(OV7670_REG_MTX6, 0x80);
    cam_hal_i2c_write(OV7670_REG_MTXS, 0x9E);

    /* Gamma curve (Linux driver values). The sensor's default is
     * near-linear and crushes shadows / blooms highlights. */
    static const uint8_t gamma_curve[16] = {
        0x20, 0x10, 0x1E, 0x35, 0x5A, 0x69, 0x76, 0x80,
        0x88, 0x8F, 0x96, 0xA3, 0xAF, 0xC4, 0xD7, 0xE8
    };
    for (uint32_t i = 0; i < 16; i++) {
        cam_hal_i2c_write(OV7670_REG_SLOP + i, gamma_curve[i]);
    }

    /* AWB seed. Power-on defaults bias blue; if AWB locks onto a flat
     * neutral-less scene, the bias sticks. Seed neutral values first. */
    cam_hal_i2c_write(OV7670_REG_BLUE, 0x40);
    cam_hal_i2c_write(OV7670_REG_RED,  0x60);

    cam_hal_i2c_write(OV7670_REG_AWBC1,   0x0A);
    cam_hal_i2c_write(OV7670_REG_AWBC2,   0xF0);
    cam_hal_i2c_write(OV7670_REG_AWBC3,   0x34);
    cam_hal_i2c_write(OV7670_REG_AWBC4,   0x58);
    cam_hal_i2c_write(OV7670_REG_AWBC5,   0x28);
    cam_hal_i2c_write(OV7670_REG_AWBC6,   0x3A);
    cam_hal_i2c_write(OV7670_REG_AWBC7,   0x88);
    cam_hal_i2c_write(OV7670_REG_AWBC8,   0x88);
    cam_hal_i2c_write(OV7670_REG_AWBC9,   0x44);
    cam_hal_i2c_write(OV7670_REG_AWBC10,  0x67);
    cam_hal_i2c_write(OV7670_REG_AWBC11,  0x49);
    cam_hal_i2c_write(OV7670_REG_AWBC12,  0x0E);
    cam_hal_i2c_write(OV7670_REG_AWBCTR3, 0x0A);
    cam_hal_i2c_write(OV7670_REG_AWBCTR2, 0x55);
    cam_hal_i2c_write(OV7670_REG_AWBCTR1, 0x11);
    cam_hal_i2c_write(OV7670_REG_AWBCTR0, 0x9F);
    cam_hal_i2c_write(OV7670_REG_GGAIN,   0x40);
    cam_hal_i2c_write(OV7670_REG_BLUE,    0x40);            /* re-seed post-AWB */
    cam_hal_i2c_write(OV7670_REG_RED,     0x60);

    cam_hal_i2c_write(OV7670_REG_BRIGHT,  0x00);
    cam_hal_i2c_write(OV7670_REG_CONTRAS, 0x40);

    /* Undocumented but well-known: this write fixes red-channel
     * suppression on virtually every OV7670 module. */
    cam_hal_i2c_write(OV7670_REG_MAGIC_B0, 0x84);

    cam_hal_delay_ms(50);
}
