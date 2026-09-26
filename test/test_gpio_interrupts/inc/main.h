#pragma once

#include <stdint.h>
#include "stm32h7xx_hal.h"

#define BUTTON_USER_PIN    GPIO_PIN_1
#define BUTTON_GPIOX       GPIOB

#define BUTTON_GPIOX_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()

int main(void);
