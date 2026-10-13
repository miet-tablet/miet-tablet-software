#pragma once

#include <stdint.h>
#include "stm32h7xx_hal.h"

#define BUTTON_1           GPIO_PIN_1
#define BUTTON1_GPIO       GPIOB
#define BUTTON_2           GPIO_PIN_2
#define BUTTON2_GPIO       GPIOB

#define BUTTON1_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define BUTTON2_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()

#define BUTTON1_IRQHandler_Config() EXTI1_IRQHandler_Config()
#define BUTTON2_IRQHandler_Config() EXTI2_IRQHandler_Config()

#define BUTTON1_IRQn EXTI1_IRQn
#define BUTTON2_IRQn EXTI2_IRQn

int main(void);
