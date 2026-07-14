/*
 * camera_hal_same54.c — HAL implementation for ATSAME54P20A 144-pin LQFP.
 *
 * Wiring:
 *   D0..D7  -> PB00..PB07 (byte 0 of PORTB IN)
 *   PCLK    -> PB16 / EIC EXTINT[0]  (falling, event-only)
 *   HSYNC   -> PB14 / EIC EXTINT[14] (rising, CPU IRQ)
 *   VSYNC   -> PB15 / EIC EXTINT[15] (rising, CPU IRQ)
 *   XCLK    -> PA04 / TC0 WO0, ~16 MHz
 *   RESET   -> PA05 (GPIO output, active-low)
 *   SCCB    -> SERCOM3 I2C (PA22=SDA PAD0, PA23=SCL PAD1, 100 kHz)
 *   UART    -> SERCOM2 USART (PB24=TX PAD0, PB25=RX PAD1, 460800 baud)
 *   SW0     -> PB31 (GPIO input, pull-up, active-low)
 *
 * Harmony's SYS_Initialize() brings up CLOCK, PORT, SERCOM3_I2C, EVSYS
 * (channel 0 gen = EIC_EXTINT_0, user 5 = DMAC_CH_0), DMAC (channel 0
 * event-triggered burst, byte, DSTINC, TCMPL IRQ), and EIC (line 0 FALL
 * event-out, lines 14/15 RISE CPU-IRQ, glitch filter on HSYNC/VSYNC).
 * This shim only wires per-frame DMA transfers and EIC callbacks.
 */

#include "camera_hal.h"
#include "camera_config.h"
#include "config/default/definitions.h"

bool cam_hal_i2c_write(uint8_t reg_addr, uint8_t reg_value)
{
    uint8_t buf[2] = { reg_addr, reg_value };

    while (SERCOM3_I2C_IsBusy()) { }
    bool ok = SERCOM3_I2C_Write((uint16_t)OV7670_I2C_ADDR_7BIT, buf, sizeof buf);
    while (SERCOM3_I2C_IsBusy()) { }

    /* Some OV7670 modules NAK back-to-back SCCB writes without a settle gap. */
    for (volatile uint32_t i = 0; i < 300000U; i++) { }
    return ok;
}

void cam_hal_reset_assert(void)   { OV_RESET_Clear(); }
void cam_hal_reset_deassert(void) { OV_RESET_Set();   }

void cam_hal_delay_ms(uint32_t ms)
{
    /* ~5 cycles per inner iter on Cortex-M4; approximate is fine for
     * sensor-init delays. */
    const uint32_t iters_per_ms = CPU_CLOCK_FREQUENCY / (1000U * 5U);
    for (uint32_t m = 0; m < ms; m++) {
        for (volatile uint32_t i = 0; i < iters_per_ms; i++) { }
    }
}

void cam_hal_mclk_start(void) { TC0_CompareStart(); }
void cam_hal_mclk_stop(void)  { TC0_CompareStop();  }

const volatile uint8_t *cam_hal_port_in_byte(void)
{
    return (const volatile uint8_t *)&PORT_REGS->GROUP[1].PORT_IN;
}

static cam_hal_dma_done_cb_t s_dma_done_cb;

static void plib_dma_cb(DMAC_TRANSFER_EVENT event, uintptr_t ctx)
{
    (void)ctx;
    if ((event == DMAC_TRANSFER_EVENT_COMPLETE ||
         event == DMAC_TRANSFER_EVENT_ERROR) && s_dma_done_cb != NULL) {
        s_dma_done_cb();
    }
}

void cam_hal_dma_configure_byte_capture(uint8_t *dest_buf, uint32_t len)
{
    DMAC_ChannelCallbackRegister((DMAC_CHANNEL)CAM_DMA_CHANNEL, plib_dma_cb, 0);
    (void)DMAC_ChannelTransfer((DMAC_CHANNEL)CAM_DMA_CHANNEL,
                               (const void *)cam_hal_port_in_byte(),
                               dest_buf,
                               (size_t)len);
}

void cam_hal_dma_start(void)
{
    /* DMAC_ChannelTransfer already enables the channel; kept for HAL symmetry. */
}

void cam_hal_dma_stop(void)
{
    DMAC_ChannelDisable((DMAC_CHANNEL)CAM_DMA_CHANNEL);
}

void cam_hal_dma_register_block_complete_cb(cam_hal_dma_done_cb_t cb)
{
    s_dma_done_cb = cb;
}

static cam_hal_isr_cb_t s_hsync_cb;
static cam_hal_isr_cb_t s_vsync_cb;

static void plib_hsync_trampoline(uintptr_t ctx) { (void)ctx; if (s_hsync_cb) s_hsync_cb(); }
static void plib_vsync_trampoline(uintptr_t ctx) { (void)ctx; if (s_vsync_cb) s_vsync_cb(); }

void cam_hal_eic_register_hsync_cb(cam_hal_isr_cb_t cb)
{
    s_hsync_cb = cb;
    EIC_CallbackRegister((EIC_PIN)CAM_EIC_HSYNC_LINE, plib_hsync_trampoline, 0);
    EIC_InterruptEnable((EIC_PIN)CAM_EIC_HSYNC_LINE);
}

void cam_hal_eic_register_vsync_cb(cam_hal_isr_cb_t cb)
{
    s_vsync_cb = cb;
    EIC_CallbackRegister((EIC_PIN)CAM_EIC_VSYNC_LINE, plib_vsync_trampoline, 0);
    EIC_InterruptEnable((EIC_PIN)CAM_EIC_VSYNC_LINE);
}

/* Cortex-M4 has no D-cache. */
void cam_hal_dcache_sync_after_capture(void *addr, uint32_t len)
{
    (void)addr;
    (void)len;
}
