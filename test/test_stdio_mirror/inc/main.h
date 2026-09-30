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

void MX_GPIO_Init(void);
void MX_RTC_Init(void);
void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle);
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle);
int main(void);

#if defined(__cplusplus)
}
#endif