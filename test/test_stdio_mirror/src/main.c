#include "main.h"

static char strbuf[50];

int main(void)
{
    MPU_Config();
	HAL_Init();
	SystemClock_Config();
    stdio_uart_init();

	

	uint32_t i;
	while (1)
	{
		printf("Hello, enter something via terminal and I'll repeat it\n> ");
		// char c = getchar();
		// putchar(c);
	}
}