#pragma once 

#include <stdint.h>
#include "controls.h"
#include "crc8.h"

typedef struct {
    uint8_t start_byte1,
    uint8_t start_byte2,
    uint8_t addr_type, 
    uint8_t buttons1, // btn7 - btn0 
    uint8_t buttons2, // 000000xx
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
    uint16_t gpio_data[4] = {0};
    get_gpio_data(GPIO_Ports_1, GPIO_Pins_1, gpio_data, 4);   
    data.buttons1 = 0;
    for (int i = 0; i < 4; i++)
    {
        data.buttons1 |= (gpio_data[i] << i);
    }
    get_gpio_data(GPIO_Ports_2, GPIO_Pins_2, gpio_data, 4);   
    for (int i = 0; i < 4; i++)
    {
        data.buttons1 |= (gpio_data[i] << i);
    }
    uint16_t hw504_btn[2] = {0};
    get_gpio_data(GPIO_Ports_3, GPIO_Pins_3, hw504_btn, 2);   
    data.buttons2 = 0;
    for (int i = 0; i < 2; i++)
    {
        data.buttons2 |= (hw504_btn[i] << i);
    }
    uint16t x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    adc_get_value_4ch(hadc1, &x1, &y1, &x2, &y2, 10); // channel priority decreases
    data.adc1x1 = 0;
    data.adc1x1 |= x1;
    data.adc1y1_adc1_x2 = (y1 << 4) | (x1 >> 8);
    data.adc1y2 = 0;
    data.adc1y2 |= x1;
    





}

void bluetooth_transmit_frame(frame data)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, 12, 1000);
}

