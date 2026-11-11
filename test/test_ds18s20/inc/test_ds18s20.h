#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include "stm32h7xx_hal.h"

#include "clock_config.h"
#include "mpu_config.h"
#include "stdio_uart.h"
#include "ds18s20.h"

void MX_GPIO_Init(void);
int main(void);

#if defined(__cplusplus)
}
#endif
