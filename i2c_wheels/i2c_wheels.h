#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "stm32h7xx_hal_i2c.h"

void i2c_wheels_set_i2c_host(I2C_HandleTypeDef *host);

int i2c_wheels_read(uint8_t *value, int len);

int i2c_wheels_write(const uint8_t *value, int len);

#if defined(__cplusplus)
}
#endif