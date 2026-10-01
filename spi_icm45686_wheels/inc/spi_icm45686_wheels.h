#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "stm32h7xx_hal.h"

//void spi_icm45686_0_set_spi_host(SPI_Type *host);
//void spi_icm45686_1_set_spi_host(SPI_Type *host);
int spi_icm45686_init(void);
int icm45686_read_register(uint8_t address, uint8_t *value, int len);
int icm45686_write_register(uint8_t address, const uint8_t *value, int len);

#if defined(__cplusplus)
}
#endif