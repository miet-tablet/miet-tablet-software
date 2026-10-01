#include "main.h"

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

UART_HandleTypeDef huart;

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(huart->Instance == UART7)
    {
        __HAL_RCC_UART7_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_UART7;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    }
}

void MX_UART7_UART_Init(void)
{
    huart.Instance = UART7;
    huart.Init.BaudRate = 115200;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart) != HAL_OK)
    {
        Error_Handler();
    }
}

void my_putc(char c)
{
    HAL_UART_Transmit(&huart, (uint8_t*)&c, 1, 1000);
}

char my_getc(void)
{
	return '0';
}

#include <stdio.h>

int main(void)
{
    MPU_Config();
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
    MX_UART7_UART_Init();
	MX_RTC_Init();

	while (1)
	{
		// HAL_UART_Transmit(&huart, (uint8_t*)"Hello\n", 6, 1000);
        printf("Hello\n");
		// fwrite("Hello\n", 1, 6, stdout);
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
		HAL_Delay(500);
	}
}