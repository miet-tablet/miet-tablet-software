* crc8 rx_tx test
* board stm32f401ccu6
* rx module
	receives bytes via uart interrupts USART1 9600 8N1
	and calculates crc8 with received checksum
	result transmitted via USB FS
* status: in progress