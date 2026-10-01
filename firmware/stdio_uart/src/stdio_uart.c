#include "stdio_uart.h"

UART_HandleTypeDef uart7_host;

void HAL_UART_MspInit(UART_HandleTypeDef *uart7_host)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uart7_host->Instance == UART7)
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

void stdio_uart_init(void)
{
    uart7_host.Instance = UART7;
    uart7_host.Init.BaudRate = 1000000;
    uart7_host.Init.WordLength = UART_WORDLENGTH_8B;
    uart7_host.Init.StopBits = UART_STOPBITS_1;
    uart7_host.Init.Parity = UART_PARITY_NONE;
    uart7_host.Init.Mode = UART_MODE_TX_RX;
    uart7_host.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart7_host.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&uart7_host) != HAL_OK)
    {
        //Error_Handler();
    }
}

void stdio_uart_putc(char c)
{
    HAL_UART_Transmit(&uart7_host, (uint8_t*)&c, 1, HAL_MAX_DELAY);
}

char stdio_uart_getc(void)
{
    uint8_t dummy;
	HAL_UART_Receive(&uart7_host, &dummy, 1, HAL_MAX_DELAY);
    return (char)dummy;
}
