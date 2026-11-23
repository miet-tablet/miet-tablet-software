#include "controls.h"

GPIO_TypeDef *GPIO_Ports_1[] =
{ GPIOA, GPIOA, GPIOA, GPIOA, GPIOA };
uint16_t GPIO_Pins_1[] =
{ GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5 };

GPIO_TypeDef *GPIO_Ports_2[] =
{ GPIOA, GPIOA, GPIOA, GPIOA, GPIOA };
uint16_t GPIO_Pins_2[] =
{ GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5 };

void adc_get_value_2ch(ADC_HandleTypeDef *hadc, uint16_t *ch1, uint16_t *ch2,
		uint32_t timeout)
{
	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, timeout);
	*ch1 = HAL_ADC_GetValue(hadc);
	HAL_Delay(1);
	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, timeout);
	*ch2 = HAL_ADC_GetValue(hadc);
	HAL_Delay(1);
}


void adc_get_value_4ch(ADC_HandleTypeDef *hadc, uint16_t *ch1, uint16_t *ch2,
		uint16_t *ch3, uint16_t *ch4, uint32_t timeout)
{
    for (int i = 0; i < 4; i++)
    {
        HAL_ADC_Start(hadc);
        HAL_ADC_PollForConversion(hadc, timeout);   
        switch (i)
        {
            case 0:
	            *ch1 = HAL_ADC_GetValue(hadc);
                break;
            case 1:
	            *ch2 = HAL_ADC_GetValue(hadc);
                break;
            case 2:
	            *ch3 = HAL_ADC_GetValue(hadc);
                break;
            case 3:
	            *ch4 = HAL_ADC_GetValue(hadc);
                break;
        }
        HAL_Delay(1);
    }
}

void get_gpio_data(GPIO_TypeDef *GPIO_Port[], uint16_t *GPIO_Pin,
		uint16_t *dest, uint16_t len)
{
	for (int i = 0; i < len; i++)
	{
		dest[i] = HAL_GPIO_ReadPin(GPIO_Port[i], GPIO_Pin[i]);
	}
}
