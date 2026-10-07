#include "app_ml.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "camera_capture.h"
#include "camera_config.h"
#include "img_preprocess.h"

extern "C" {
    #include "TinyEngine/include/genNN.h"
}

#define CPU_FREQ_HZ  120000000UL

#define PERSON_EMA_ALPHA_SHIFT  2
#define PERSON_ENTER_Q8         ( 4 * 256)
#define PERSON_EXIT_Q8          (-8 * 256)

static volatile bool    s_person_present = false;
static volatile int8_t  s_person_logit = 0;
static volatile int8_t  s_noperson_logit = 0;
static volatile uint32_t s_inference_us = 0;
static volatile uint8_t  s_inference_count = 0;
static volatile int16_t  s_margin_ema_q8 = 0;
static bool s_initialized = false;
static bool s_frame_processed = false;

static inline void dwt_enable(void)
{
    volatile uint32_t *demcr  = (volatile uint32_t *)0xE000EDFC;
    volatile uint32_t *cyccnt = (volatile uint32_t *)0xE0001004;
    volatile uint32_t *ctrl   = (volatile uint32_t *)0xE0001000;
    *demcr  |= (1u << 24);
    *cyccnt  = 0;
    *ctrl   |= 1u;
}

static inline uint32_t dwt_get_cyccnt(void)
{
    return *(volatile uint32_t *)0xE0001004;
}

static void run_person_classification(const uint8_t *frame)
{
    signed char* in = getInput();
    rgb565_to_modelinput_vww(frame, in,
                             CAM_FRAME_WIDTH, CAM_FRAME_HEIGHT);

    uint32_t c0 = dwt_get_cyccnt();
    invoke(NULL);
    uint32_t c1 = dwt_get_cyccnt();
    s_inference_us = (uint32_t)(((uint64_t)(c1 - c0) * 1000000ULL) / CPU_FREQ_HZ);

    const signed char* out = getOutput();
    int8_t noperson_logit = out[0];
    int8_t person_logit   = out[1];

    s_person_logit   = person_logit;
    s_noperson_logit = noperson_logit;

    int16_t margin = (int16_t)((int)person_logit - (int)noperson_logit);

    int16_t raw_q8 = (int16_t)(margin * 256);
    s_margin_ema_q8 += (int16_t)((raw_q8 - s_margin_ema_q8) >> PERSON_EMA_ALPHA_SHIFT);

    bool present = s_person_present;
    if (!present && s_margin_ema_q8 >= PERSON_ENTER_Q8)      present = true;
    else if (present && s_margin_ema_q8 <= PERSON_EXIT_Q8)   present = false;
    s_person_present = present;

    printf("Person: %s  logits=[%d, %d]  margin=%d  inf_us=%lu\r\n",
           present ? "YES" : "no ",
           (int)person_logit, (int)noperson_logit, (int)margin,
           (unsigned long)s_inference_us);

    s_inference_count = (uint8_t)(s_inference_count + 1u);
}

void APP_ML_Initialize(void)
{
    dwt_enable();
    s_initialized = true;
    printf("[ML] TinyEngine VWW initialized\r\n");
}

void APP_ML_Tasks(void)
{
    if (!s_initialized) return;

    if (!camera_capture_frame_ready()) {
        s_frame_processed = false;
        return;
    }
    if (s_frame_processed) return;

    camera_capture_pause();
    const uint8_t *frame = camera_capture_get_frame();
    run_person_classification(frame);
    s_frame_processed = true;

#if !USB_STREAM_ENABLED
    camera_capture_release_frame();
    camera_capture_unpause();
#endif
}

bool APP_ML_GetPersonPresent(void)
{
    return s_person_present;
}

int8_t APP_ML_GetPersonLogit(void)
{
    return (int8_t)s_person_logit;
}

int8_t APP_ML_GetNoPersonLogit(void)
{
    return (int8_t)s_noperson_logit;
}

uint32_t APP_ML_GetInferenceUs(void)
{
    return s_inference_us;
}

uint8_t APP_ML_GetInferenceCount(void)
{
    return s_inference_count;
}
