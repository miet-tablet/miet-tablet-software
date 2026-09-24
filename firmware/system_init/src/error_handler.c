#include "error_handler.h"

void Error_Handler(void)
{
	__disable_irq();
	while (1);
}
