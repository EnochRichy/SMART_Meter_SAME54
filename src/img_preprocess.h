#ifndef IMG_PREPROCESS_H
#define IMG_PREPROCESS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODEL_IN_H  64
#define MODEL_IN_W  64
#define MODEL_IN_C  3
#define MODEL_IN_BYTES (MODEL_IN_H * MODEL_IN_W * MODEL_IN_C)

void rgb565_to_modelinput_vww(const uint8_t *src, signed char *dst,
                              int src_w, int src_h);

#ifdef __cplusplus
}
#endif

#endif /* IMG_PREPROCESS_H */
