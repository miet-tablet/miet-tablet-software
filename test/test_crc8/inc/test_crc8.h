/**
 * @file    main.h
 * @brief   Общие настройки теста crc8: светодиод и ожидаемые значения CRC
 *
 * @details Тест показывает результат светодиодом.
 */

#pragma once

#include <stdint.h>
#include "stm32h7xx_hal.h"

#include "clock_config.h"
#include "mpu_config.h"
#include "crc8.h"

#define LED_PORT          GPIOE
#define LED_PIN           GPIO_PIN_3
#define LED_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()

/* ---------- Ожидаемые значения CRC ---------- */

/** @brief CRC пустых данных (равна начальному значению) */
#define TEST_CRC8_EXPECT_EMPTY      0x00

/** @brief CRC одного байта 0x00 */
#define TEST_CRC8_EXPECT_BYTE_00    0x00

/** @brief CRC одного байта 0x01 */
#define TEST_CRC8_EXPECT_BYTE_01    0x5E

/** @brief CRC одного байта 0x80 */
#define TEST_CRC8_EXPECT_BYTE_80    0x8C

/** @brief CRC одного байта 0xFF */
#define TEST_CRC8_EXPECT_BYTE_FF    0x35

/** @brief CRC стандартной строки "123456789" для CRC-8/MAXIM-DOW */
#define TEST_CRC8_EXPECT_CHECK_STR  0xA1

/** @brief CRC первых 7 байт ROM-кода 1-Wire (Maxim AN27) */
#define TEST_CRC8_EXPECT_ROM        0xA2

/** @brief CRC данных вместе с их собственной CRC всегда равна нулю */
#define TEST_CRC8_EXPECT_WITH_CRC   0x00

int main(void);
