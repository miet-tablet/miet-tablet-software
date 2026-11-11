#include "app_main.h"

void app_main(void)
{
	char message[MSG_MAX_SIZE] =
	{ 0, };
	uint32_t size_bytes;
	while (1)
	{
		size_bytes = 0;
		scanf("%s", message);
		while (message[size_bytes] != 0)
			size_bytes++;
		uint8_t crc = crc8_calc((uint8_t*) message, size_bytes);
		message[size_bytes++] = crc;
		message[size_bytes] = 0;
		for (int i = 0; i < size_bytes + 1; i++)
		{
			uint8_t tx_data = message[i];
			HAL_UART_Transmit(&huart2, &tx_data, 1, HAL_MAX_DELAY);
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			HAL_Delay(1000);
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			HAL_Delay(2000);
		}
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
		Error_Handler();
		printf("%s", message);
		Error_Handler();
	}
}
