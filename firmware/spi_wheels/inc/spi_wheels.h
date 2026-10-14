#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "stm32h7xx_hal.h"

int init(void);
int read_register(uint8_t address, uint8_t *value, int len);
int write_register(uint8_t address, const uint8_t *value, int len);

#if defined(__cplusplus)
}
#endif