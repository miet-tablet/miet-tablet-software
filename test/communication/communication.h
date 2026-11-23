#pragma once 

#include <stdint.h>
#include "controls.h"
#include "crc8.h"

typedef struct {
    uint8_t start_byte1,
    uint8_t start_byte2,
    uint8_t addr_type, 
    uint8_t buttons1, 
    uint8_t buttons2,
    uint8_t adc1x1,
    uint8_t adc1y1_adc1x2,
    uint8_t adc1y2,
    uint8_t adc2x1,
    uint8_t adc2y1_adc2x2,
    uint8_t adc2y2,
    uint8_t crc8

} frame;

typedef enum {
    dest1 0x50, // 0101
    dest2 0xA0, // 1010
    dest3 0x30 // 0011

} dest_addr;


frame make_frame_gpio(dest_addr dst);

frame make_frame_touchscreen(dest_addr dst);

void bluetooth_transmit_frame(frame data);
