#pragma once

#include <stdint.h>
#include <stdio.h>
#include "stm32h7xx_hal.h"

#include "clock_config.h"
#include "mpu_config.h"
#include "stdio_uart.h"

void MX_GPIO_Init(void);
void MX_I2C2_Init(void);

int main(void);
