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


frame make_frame()
{

}

void bluetooth_transmit_frame(frame data)
{
    HAL_UART_TRANSMIT(&huart1, (uint8_t *)data, 12, 1000);
}

