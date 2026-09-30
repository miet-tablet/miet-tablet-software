#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "stm32h7xx_hal.h"

void HAL_UART_MspInit(UART_HandleTypeDef *uart7_host);
void stdio_uart_init(void);
void stdio_uart_putc(char c);
char stdio_uart_getc(void);

#if defined(__cplusplus)
}
#endif