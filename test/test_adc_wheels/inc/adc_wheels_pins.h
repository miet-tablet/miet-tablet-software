#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Заголовок с аппаратными настройками для ADC-джойстиков.
 * Здесь собраны все пины и каналы, чтобы их было легко поправить
 * при изменении платы, без правки main.c и msp.c.
 */

#include "stm32h7xx_hal.h"

/* --- Какой ADC используем --- */
#define ADC_WHEELS_INSTANCE          ADC1

/* --- Оси джойстика 1 --- */
#define ADC_WHEELS_AXIS_X_CHANNEL    ADC_CHANNEL_0
#define ADC_WHEELS_AXIS_X_PORT       GPIOA
#define ADC_WHEELS_AXIS_X_PIN        GPIO_PIN_0

#define ADC_WHEELS_AXIS_Y_CHANNEL    ADC_CHANNEL_1
#define ADC_WHEELS_AXIS_Y_PORT       GPIOA
#define ADC_WHEELS_AXIS_Y_PIN        GPIO_PIN_1

/* --- Оси джойстика 2 (если есть) --- */
#define ADC_WHEELS_AXIS_X2_CHANNEL   ADC_CHANNEL_2
#define ADC_WHEELS_AXIS_X2_PORT      GPIOA
#define ADC_WHEELS_AXIS_X2_PIN       GPIO_PIN_2

#define ADC_WHEELS_AXIS_Y2_CHANNEL   ADC_CHANNEL_3
#define ADC_WHEELS_AXIS_Y2_PORT      GPIOA
#define ADC_WHEELS_AXIS_Y2_PIN       GPIO_PIN_3

/* --- Время сэмплирования --- */
#define ADC_WHEELS_SAMPLING_TIME     ADC_SAMPLETIME_64CYCLES_5

#if defined(__cplusplus)
}
#endif
