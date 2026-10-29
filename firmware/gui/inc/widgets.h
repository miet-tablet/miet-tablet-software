#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "gfx.h"

/*
 * Уровень 2: элементы интерфейса. Каждый рисуется по координатам и состоянию,
 * сам ничего не хранит. Цвета общие для всего интерфейса.
 */

#define WG_COLOR_BG        GFX_WHITE
#define WG_COLOR_TEXT      GFX_BLACK
#define WG_COLOR_BORDER    GFX_BLACK
#define WG_COLOR_PANEL     GFX_RGB(216, 216, 216)
#define WG_COLOR_SELECTED  GFX_RGB(150, 200, 255)
#define WG_COLOR_PRESSED   GFX_RGB(130, 130, 130)
#define WG_COLOR_ALERT     GFX_RGB(220, 0, 0)

#define WG_BUTTONS     4   /* кнопок в крестовине: вверх, влево, вправо, вниз */
#define WG_JOYCONS_W   (2 * GFX_JOYCON_W + 3)

typedef enum
{
    WG_ICON_OFF,    /* значка нет */
    WG_ICON_BLINK,  /* мигает */
    WG_ICON_ON,     /* горит */
} wg_icon_t;

void wg_clear(gfx_rect_t r);

/* Надписи. Фон под надписью заливается цветом bg */
void wg_label(gfx_rect_t r, const char *text, gfx_align_t align, uint16_t color, uint16_t bg);
void wg_clock(gfx_rect_t r, uint32_t seconds, uint16_t bg);

/* Верх экрана */
void wg_status_bar(gfx_rect_t r);
void wg_vehicle_panel(gfx_rect_t r, const char *name, const char *summary, bool selected);
void wg_tab(gfx_rect_t r, bool up);

/*
 * Пунктирные рамки. wg_text_lines обновляет только текст внутри уже
 * нарисованной рамки: строки разделяются '\n', лишние обрезаются.
 */
void wg_text_box(gfx_rect_t r, const char *text);
void wg_text_lines(gfx_rect_t r, const char *text);
void wg_assign_list(gfx_rect_t r, gfx_side_t side,
                    const char *const buttons[WG_BUTTONS], const char *stick);

/* Экранное управление */
void wg_round_button(uint16_t cx, uint16_t cy, uint8_t radius, gfx_dir_t dir, bool pressed);
void wg_stick(uint16_t cx, uint16_t cy, uint8_t radius, bool pressed);
void wg_logo(uint16_t cx, uint16_t cy, uint8_t radius);

/* Значки состояния. Невидимый значок стирается фоном bg */
bool wg_icon_visible(wg_icon_t state, bool blink_on);
void wg_joycons(uint16_t x, uint16_t y, bool left, bool right, uint16_t bg);
void wg_bluetooth(uint16_t x, uint16_t y, bool visible, uint16_t bg);
void wg_battery(uint16_t x, uint16_t y, uint8_t level, uint16_t bg);

/* Попало ли касание (x, y) в элемент */
bool wg_hit(gfx_rect_t r, uint16_t x, uint16_t y);
bool wg_hit_circle(uint16_t cx, uint16_t cy, uint8_t radius, uint16_t x, uint16_t y);

#if defined(__cplusplus)
}
#endif
