#pragma once 

#include <stdint.h>
#include "controls.h"
#include "crc8.h"

typedef struct {
    uint8_t start_byte1,
    uint8_t start_byte2,
    uint8_t addr_type, 
    uint8_t buttons1, // btn7 - btn1 
    uint8_t buttons2, // 00000xxx
    uint8_t adc1x1,
    uint8_t adc1y1_adc1x2, // yyyyxxxx
    uint8_t adc1y2,
    uint8_t adc2x1,
    uint8_t adc2y1_adc2x2,
    uint8_t adc2y2,
    uint8_t crc8

} frame;

typedef enum {
    dest1 0x05, // 0101
    dest2 0x0A, // 1010
    dest3 0x03 // 0011

} dest_addr; // last 4 bits are valid


frame make_frame_tx(dest_addr dst)
{
    frame data;
    data.start_byte1 = 0x6A;
    data.start_byte2 = 0x1E;   
    data.addr_type = 0;
    data.addr_type |= (dst << 4);
    uint16_t buttons1[4] = {0};
    get_gpio_data();   
}

void bluetooth_transmit_frame(frame data)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, 12, 1000);
}

