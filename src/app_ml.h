#ifndef _APP_ML_H
#define _APP_ML_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void APP_ML_Initialize(void);
void APP_ML_Tasks(void);

bool    APP_ML_GetPersonPresent(void);
int8_t  APP_ML_GetPersonLogit(void);
int8_t  APP_ML_GetNoPersonLogit(void);
uint32_t APP_ML_GetInferenceUs(void);
uint8_t  APP_ML_GetInferenceCount(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_ML_H */
