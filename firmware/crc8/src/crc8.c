/**
 * @file    crc8.c
 * @brief   Реализация подсчёта CRC-8 (Dallas/Maxim)
 */

#include "crc8.h"

/**
 * @brief   Зеркальный (reflected) полином для CRC-8 Dallas/Maxim
 *
 * @details Обычный полином x^8 + x^5 + x^4 + 1 пишут как 0x31
 *          (старший бит x^8 не записывают). Но у нас данные идут
 *          младшим битом вперёд, поэтому биты полинома надо
 *          развернуть: 0x31 = 0011 0001 -> 1000 1100 = 0x8C
 */
#define CRC8_POLY_REFLECTED 0x8C

uint8_t crc8_update(uint8_t crc, uint8_t data)
{
    // Сначала смешиваем новый байт с текущей CRC
    crc = crc ^ data;

    // Дальше по очереди гоняем все 8 бит
    for (uint8_t bit = 0; bit < 8; bit++)
    {
        if (crc & 0x01)
        {
            // Младший бит = 1: сдвигаем вправо и делаем XOR с полиномом
            crc = (crc >> 1) ^ CRC8_POLY_REFLECTED;
        }
        else
        {
            // Младший бит = 0: просто сдвигаем вправо
            crc = crc >> 1;
        }
    }

    return crc;
}

uint8_t crc8_calc(const uint8_t *data, uint32_t length)
{
    uint8_t crc = CRC8_INIT_VALUE;

    // Если указатель нулевой, то выходим
    if (data == 0)
    {
        return crc;
    }

    // Прогоняем через crc8_update() каждый байт массива
    for (uint32_t i = 0; i < length; i++)
    {
        crc = crc8_update(crc, data[i]);
    }

    return crc;
}
