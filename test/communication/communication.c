#include "communication.h"

frame make_frame_gpio(dest_addr dst)
{
    frame data = {0};
    data.start_byte1 = 0x6A;
    data.start_byte2 = 0x1E;   
    data.addr_type = dst;
    uint16_t gpio_data[4] = {0};
    get_gpio_data(GPIO_Ports_1, GPIO_Pins_1, gpio_data, 4);   
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
    for (int i = 0; i < 2; i++)
    {
        data.buttons2 |= (hw504_btn[i] << i);
    }
    uint16t adc1x = 0, adc1y = 0, adc2x = 0, adc2y = 0;
    adc_get_value_4ch(hadc1, &adc1x, &adc1y, &adc2x, &adc2y, 10);
    data.adc1x1 |= adc1x;
    data.adc1y1_adc1x2 = (adc1y << 4) | (adc1x >> 8);
    data.adc1y2 |= adc1y >> 4;
    data.adc2x1 = adc2x;
    data.adc2y1_adc2x2 |= (adc2y << 4) | (adc2x >> 8);
    data.adc2y2 |= adc1y >> 4;
    uint8_t crc_data[] = {
                          start_byte1,
                          start_byte2,
                          addr_type, 
                          buttons1, 
                          buttons2,
                          adc1x1,
                          adc1y1_adc1x2,
                          adc1y2,
                          adc2x1,
                          adc2y1_adc2x2,
                          adc2y2,
    };

    uint8_t crc = crc8_calc(crc_data, 11);
    data.crc8 = crc;
    return data;
}

frame make_frame_touchscreen(dest_addr dst);

void bluetooth_transmit_frame(frame data)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, 12, 1000);
}

