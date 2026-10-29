#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "stm32h7xx_hal.h"

#include "clock_config.h"
#include "mpu_config.h"
#include "gui.h"
#include "touch_gsl1680.h"

/* Светодиод платы WeAct (PE3) */
#define LED_PORT          GPIOE
#define LED_PIN           GPIO_PIN_3
#define LED_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()

int main(void);
