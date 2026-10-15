#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "ra8875_regs.h"

/*
 * Display wiring (see README.md): SPI2 with a software chip select.
 * The same pins are used for tests on the DevEBox STM32H7XX_M board.
 * To move the display to another SPI, change the instance, AF and pins here.
 */
#define RA8875_SPI                    SPI2
#define RA8875_SPI_CLK_ENABLE()       __HAL_RCC_SPI2_CLK_ENABLE()
#define RA8875_SPI_AF                 GPIO_AF5_SPI2
#define RA8875_SPI_GPIO_PORT          GPIOB
#define RA8875_SPI_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define RA8875_SCK_PIN                GPIO_PIN_13
#define RA8875_MISO_PIN               GPIO_PIN_14
#define RA8875_MOSI_PIN               GPIO_PIN_15
#define RA8875_CS_PORT                GPIOB
#define RA8875_CS_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()
#define RA8875_CS_PIN                 GPIO_PIN_12

/*
 * Bus layer: the only part of the driver that touches the MCU peripherals.
 * ra8875.c talks to the controller through these ra8875_port_* functions,
 * so a parallel-bus version only has to provide the same set.
 */

/* Starts slow: the RA8875 can't take a fast bus until its PLL runs */
int ra8875_port_init(void);
void ra8875_port_set_fast(void);

void ra8875_port_write_cmd(uint8_t reg);
void ra8875_port_write_data(uint8_t data);
uint8_t ra8875_port_read_data(void);
uint8_t ra8875_port_read_status(void);

/* Streams RGB565 pixels as one data cycle, after an MRWC command */
void ra8875_port_write_pixels(const uint16_t *pixels, uint32_t count);

/*
 * Wiring check without a display: with MISO shorted to MOSI every byte sent
 * must come back. Returns 0 if it does.
 */
int ra8875_port_loopback_test(void);

#if defined(__cplusplus)
}
#endif
