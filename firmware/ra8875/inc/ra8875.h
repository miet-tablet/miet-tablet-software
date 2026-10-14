#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "ra8875_spi.h"

#define RA8875_WIDTH   800
#define RA8875_HEIGHT  480

/* RGB565 colours */
#define RA8875_BLACK    0x0000
#define RA8875_BLUE     0x001F
#define RA8875_RED      0xF800
#define RA8875_GREEN    0x07E0
#define RA8875_CYAN     0x07FF
#define RA8875_MAGENTA  0xF81F
#define RA8875_YELLOW   0xFFE0
#define RA8875_WHITE    0xFFFF
#define RA8875_RGB565(r, g, b) \
    ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

/*
 * Brings up the bus, checks that an RA8875 answers and starts the panel with
 * the backlight at full brightness. Returns 0 on success.
 */
int ra8875_init(void);

void ra8875_display_on(int on);
void ra8875_backlight(uint8_t level);

/* Shapes are drawn by the controller itself. (0, 0) is the top-left pixel */
void ra8875_fill_screen(uint16_t color);
void ra8875_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ra8875_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ra8875_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ra8875_draw_circle(uint16_t x, uint16_t y, uint8_t r, uint16_t color);
void ra8875_fill_circle(uint16_t x, uint16_t y, uint8_t r, uint16_t color);
void ra8875_draw_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2, uint16_t color);
void ra8875_fill_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2, uint16_t color);

/* Built-in 8x16 ASCII font. scale 0..3 means 1x..4x */
void ra8875_draw_text(uint16_t x, uint16_t y, const char *text,
                      uint16_t fg, uint16_t bg, uint8_t scale);

/* Raw RGB565 pixels going right from (x, y); keep them within one row */
void ra8875_write_pixels(uint16_t x, uint16_t y, const uint16_t *pixels, uint32_t count);

void ra8875_write_reg(uint8_t reg, uint8_t value);
uint8_t ra8875_read_reg(uint8_t reg);
uint8_t ra8875_read_status(void);

#if defined(__cplusplus)
}
#endif
