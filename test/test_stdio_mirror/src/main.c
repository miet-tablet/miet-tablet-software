#include "main.h"

static char strbuf[50];
extern UART_HandleTypeDef uart7_host;

int main(void)
{
    MPU_Config();
	HAL_Init();
	SystemClock_Config();
    stdio_uart_init();


	printf("Hello, enter something via terminal and I'll repeat it\n> ");

	while (1)
	{
		char a;
		HAL_UART_Receive(&uart7_host, (uint8_t*)&a, 1, HAL_MAX_DELAY);
		putchar(a);
	}

	uint32_t i;
	while (1)
	{
		i = 0;
		while (i < sizeof(strbuf) && strbuf[i] != '\n')
		{
			char c = getchar();
			if (c == '\n')
			{
				strbuf[i] = '\0';
			}
			else
			{
				strbuf[i] = c;
				putchar(c);
				i += 1;
			}
		}
		printf("\n> ");
	}
}
