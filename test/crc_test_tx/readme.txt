* crc8 rx_tx test
* board: stm32f103c6t6a
* tx module
	receives bytes via uart polling from USART1 as message
	calculates crc8
	appends to message with null terminator
	result transmitted via USART2 9600 8N1
	implements stdio library functions by overwriting syscalls (stdio_uart_syscalls)
* status: in progress