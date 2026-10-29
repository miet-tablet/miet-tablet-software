#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ra8875.h"

/*
 * Уровень 1: составные фигуры поверх драйвера ra8875.
 * Только рисует, состояния не хранит. (0, 0) - левый верхний пиксель.
 */

#define GFX_WIDTH   RA8875_WIDTH
#define GFX_HEIGHT  RA8875_HEIGHT

/* Символ встроенного шрифта при scale = 0; scale N увеличивает в N + 1 раз */
#define GFX_CHAR_W  8
#define GFX_CHAR_H  16

#define GFX_RGB(r, g, b)  RA8875_RGB565(r, g, b)
#define GFX_BLACK         RA8875_BLACK
#define GFX_WHITE         RA8875_WHITE

/* Размеры значков */
#define GFX_BATTERY_W    36
#define GFX_BATTERY_H    18
#define GFX_BLUETOOTH_W  13
#define GFX_BLUETOOTH_H  20
#define GFX_JOYCON_W     11
#define GFX_JOYCON_H     20

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} gfx_rect_t;

typedef enum
{
    GFX_ALIGN_LEFT,
    GFX_ALIGN_CENTER,
    GFX_ALIGN_RIGHT,
} gfx_align_t;

/* Порядок как на эскизе: вверх, влево, вправо, вниз */
typedef enum
{
    GFX_DIR_UP,
    GFX_DIR_LEFT,
    GFX_DIR_RIGHT,
    GFX_DIR_DOWN,
} gfx_dir_t;

typedef enum
{
    GFX_SIDE_LEFT,
    GFX_SIDE_RIGHT,
} gfx_side_t;

/* Запуск дисплея, 0 - успех */
int gfx_init(void);

/* Фигуры */
void gfx_clear(uint16_t color);
void gfx_fill(gfx_rect_t r, uint16_t color);
void gfx_border(gfx_rect_t r, uint16_t color);
void gfx_frame(gfx_rect_t r, uint16_t fill, uint16_t border);
void gfx_dashed_rect(gfx_rect_t r, uint16_t color);
void gfx_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void gfx_circle(uint16_t cx, uint16_t cy, uint8_t r, uint16_t fill, uint16_t border);

/*
 * Текст до '\0' или '\n'. gfx_text_in обрезает его по ширине r,
 * выравнивает и заливает фоном остаток r, поэтому перерисовка не мерцает.
 */
uint16_t gfx_text_width(const char *text, uint8_t scale);
void gfx_text_in(gfx_rect_t r, const char *text, gfx_align_t align,
                 uint16_t fg, uint16_t bg, uint8_t scale);

/* Значки: (x, y) - левый верхний угол, стрелка задаётся центром */
void gfx_arrow(uint16_t cx, uint16_t cy, uint16_t size, gfx_dir_t dir, uint16_t color);
void gfx_icon_battery(uint16_t x, uint16_t y, uint8_t level, uint16_t color, uint16_t bg);
void gfx_icon_bluetooth(uint16_t x, uint16_t y, uint16_t color);
void gfx_icon_joycon(uint16_t x, uint16_t y, gfx_side_t side, uint16_t color, uint16_t bg);

/* Заглушка логотипа МИЭТ: круг с надписью */
void gfx_logo(uint16_t cx, uint16_t cy, uint8_t r);

#if defined(__cplusplus)
}
#endif
