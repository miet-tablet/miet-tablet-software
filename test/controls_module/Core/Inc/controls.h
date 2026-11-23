#ifndef CORE_INC_CONTROLS_H_
#define CORE_INC_CONTROLS_H_

#include "main.h"
#include "adc.h"
#include "gpio.h"

extern GPIO_TypeDef *GPIO_Port[];
extern uint16_t GPIO_Pin[];

void adc_get_value_2ch(ADC_HandleTypeDef *hadc, uint16_t *ch1, uint16_t *ch2,
		uint32_t timeout);

void adc_get_value_4ch(ADC_HandleTypeDef *hadc, uint16_t *ch1, uint16_t *ch2,
		uint16_t *ch3, uint16_t *ch4, uint32_t timeout);

void get_gpio_data(GPIO_TypeDef *GPIO_Port[], uint16_t *GPIO_Pin,
		uint16_t *dest, uint16_t len);

#endif /* CORE_INC_CONTROLS_H_ */
