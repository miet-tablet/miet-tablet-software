#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "stm32h7xx_hal.h"

int spi_wheels_init(void);
int spi_wheels_read_register(uint8_t address, uint8_t *value, int len);
int spi_wheels_write_register(uint8_t address, const uint8_t *value, int len);
int spi_wheels_transfer(const uint8_t *tx, uint8_t *rx, int len);

#if defined(__cplusplus)
}
#endif