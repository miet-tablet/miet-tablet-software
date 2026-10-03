#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "stm32h7xx_hal_dma.h"
#include <stdint.h>
#include "stm32h7xx_hal_adc.h"

void adc_wheels_set_adc_host(ADC_HandleTypeDef *host);

int adc_wheels_read(uint32_t channel, uint16_t *value);

int adc_wheels_read_axis(uint8_t axis, uint16_t *value);

#if defined(__cplusplus)
}
#endif
