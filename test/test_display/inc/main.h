#pragma once

#include <stdint.h>
#include "stm32h7xx_hal.h"

#include "clock_config.h"
#include "mpu_config.h"
#include "ra8875.h"

/*
 * Status LED: user LED of the WeAct MiniSTM32H7B0 board (PE3).
 * For a test on the DevEBox STM32H7XX_M use its LED D2 instead:
 *   LED_PORT GPIOA, LED_PIN GPIO_PIN_1, LED_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
 * (on the DevEBox PE3 is the K1 button, not a LED).
 */
#define LED_PORT          GPIOE
#define LED_PIN           GPIO_PIN_3
#define LED_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()

int main(void);
