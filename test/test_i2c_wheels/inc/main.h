#pragma once

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32h7xx_hal.h"

/* Прототипы системных функций */
int main(void);
void SystemClock_Config(void);
void Error_Handler(void);

/* Прототипы функций инициализации периферии */
void MX_GPIO_Init(void);
void MX_I2C2_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
