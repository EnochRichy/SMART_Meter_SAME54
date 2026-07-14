/*
 * ov7670.h — OV7670 sensor register map + init API
 *
 * Depends only on camera_hal.h. No target-specific headers.
 */

#ifndef OV7670_H
#define OV7670_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Named register addresses (subset used by the QQVGA RGB565 init)
 *
 * See "OmniVision OV7670/OV7171 CMOS VGA CameraChip" datasheet for the full
 * register map. Only registers we actually touch are listed here.
 * ------------------------------------------------------------------------- */
#define OV7670_REG_GAIN         0x00    /* AGC gain (low byte)                  */
#define OV7670_REG_BLUE         0x01    /* AWB blue channel gain                */
#define OV7670_REG_RED          0x02    /* AWB red channel gain                 */
#define OV7670_REG_VREF         0x03    /* Vertical frame control (VREF)        */
#define OV7670_REG_COM3         0x0C    /* Common control 3 (DCW / scale enable)*/
#define OV7670_REG_COM5         0x0F    /* (reserved on many datasheets)        */
#define OV7670_REG_CLKRC        0x11    /* Internal clock prescaler             */
#define OV7670_REG_COM7         0x12    /* Format + resolution + soft reset     */
#define OV7670_REG_COM8         0x13    /* AGC / AWB / AEC enable, fast mode    */
#define OV7670_REG_COM9         0x14    /* AGC ceiling, Freeze AGC/AEC          */
#define OV7670_REG_COM10        0x15    /* PCLK/HREF/VSYNC signal polarity      */
#define OV7670_REG_HSTART       0x17    /* Horiz window start                    */
#define OV7670_REG_HSTOP        0x18    /* Horiz window stop                     */
#define OV7670_REG_VSTART       0x19    /* Vert window start                     */
#define OV7670_REG_VSTOP        0x1A    /* Vert window stop                      */
#define OV7670_REG_AEW          0x24    /* AEC upper limit (fast mode)          */
#define OV7670_REG_AEB          0x25    /* AEC lower limit                      */
#define OV7670_REG_VPT          0x26    /* AEC "fast" region                    */
#define OV7670_REG_HREF         0x32    /* HREF control                          */
#define OV7670_REG_TSLB         0x3A    /* Line buffer test / RGB byte order    */
#define OV7670_REG_COM12        0x3C    /* Common control 12                    */
#define OV7670_REG_COM13        0x3D    /* UV output saturation, gamma          */
#define OV7670_REG_COM14        0x3E    /* PCLK divider, manual scaling         */
#define OV7670_REG_COM15        0x40    /* Data format / range (RGB565 etc)     */
/* AWB advanced (undocumented on many datasheet copies; Linux driver names) */
#define OV7670_REG_AWBC1        0x43
#define OV7670_REG_AWBC2        0x44
#define OV7670_REG_AWBC3        0x45
#define OV7670_REG_AWBC4        0x46
#define OV7670_REG_AWBC5        0x47
#define OV7670_REG_AWBC6        0x48
/* Color matrix (YUV -> RGB) */
#define OV7670_REG_MTX1         0x4F
#define OV7670_REG_MTX2         0x50
#define OV7670_REG_MTX3         0x51
#define OV7670_REG_MTX4         0x52
#define OV7670_REG_MTX5         0x53
#define OV7670_REG_MTX6         0x54
#define OV7670_REG_BRIGHT       0x55
#define OV7670_REG_CONTRAS      0x56    /* CNST */
#define OV7670_REG_MTXS         0x58    /* MTX sign bits + scaling              */
#define OV7670_REG_AWBC7        0x59
#define OV7670_REG_AWBC8        0x5A
#define OV7670_REG_AWBC9        0x5B
#define OV7670_REG_AWBC10       0x5C
#define OV7670_REG_AWBC11       0x5D
#define OV7670_REG_AWBC12       0x5E
#define OV7670_REG_GGAIN        0x6A
#define OV7670_REG_AWBCTR3      0x6C
#define OV7670_REG_AWBCTR2      0x6D
#define OV7670_REG_AWBCTR1      0x6E
#define OV7670_REG_AWBCTR0      0x6F
/* Downsampling / scaling */
#define OV7670_REG_SCALING_XSC  0x70
#define OV7670_REG_SCALING_YSC  0x71
#define OV7670_REG_SCALING_DCW  0x72
#define OV7670_REG_SCALING_PCLK 0x73
/* Gamma curve (16 points) */
#define OV7670_REG_SLOP         0x7A
#define OV7670_REG_GAM_BASE     0x7B    /* GAM1..GAM15 at +0..+14 */
/* Advanced / undocumented */
#define OV7670_REG_RGB444       0x8C
#define OV7670_REG_PCLK_DELAY   0xA2
#define OV7670_REG_MAGIC_B0     0xB0    /* Undocumented — fixes red-channel     */

/* COM7 bits (0x12) */
#define OV7670_COM7_RESET       0x80    /* Soft reset                            */
#define OV7670_COM7_RGB_QVGA    0x14    /* RGB output + QVGA scaling enable      */

/* ---------------------------------------------------------------------------
 * Public API
 *
 * Both functions assume MCLK/XCLK is already being driven to the sensor and
 * the I2C bus is operational (cam_hal_i2c_write returns immediately).
 * ------------------------------------------------------------------------- */

/* Hardware reset via RESET pin, followed by a soft reset via I2C. */
void ov7670_reset(void);

/* Program the sensor for RGB565 QQVGA (160x120), auto exposure / gain / white
 * balance, sensible AGC ceiling, full color matrix, gamma curve, and the
 * "magic" 0xB0 = 0x84 fix.
 *
 * Preconditions: MCLK running, I2C bus initialised, RESET already released. */
void ov7670_init_rgb565_qqvga(void);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_H */
