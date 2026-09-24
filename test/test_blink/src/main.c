#include "main.h"

void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

RNG_HandleTypeDef hrng;

void MX_RNG_Init(void)
{
	hrng.Instance = RNG;
	hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
	if (HAL_RNG_Init(&hrng) != HAL_OK)
	{
		Error_Handler();
	}
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *rngHandle)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if(rngHandle->Instance == RNG)
	{
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
		PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_RCC_RNG_CLK_ENABLE();
	}
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef *rngHandle)
{
	if(rngHandle->Instance==RNG)
	{
		__HAL_RCC_RNG_CLK_DISABLE();
	}
}

RTC_HandleTypeDef hrtc;

void MX_RTC_Init(void)
{
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if(rtcHandle->Instance==RTC)
	{
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
		PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_RCC_RTC_ENABLE();
	}
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{
	if(rtcHandle->Instance==RTC)
	{
		__HAL_RCC_RTC_DISABLE();
	}
}

int main(void)
{
    MPU_Config();
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_RNG_Init();
	MX_RTC_Init();

	uint32_t rng;
	while (1)
	{
		HAL_RNG_GenerateRandomNumber(&hrng,&rng);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
		HAL_Delay(rng%600);
	}
}