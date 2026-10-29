#include "widgets.h"

#define PAD         8   /* отступ текста от пунктирной рамки */
#define LINE_H      20  /* строка телеметрии */
#define ASSIGN_H    27  /* строка назначения кнопки */
#define ARROW_SIZE  12
#define ARROW_GAP   8   /* между стрелкой и текстом */
#define JOYCON_GAP  (WG_JOYCONS_W - 2 * GFX_JOYCON_W)

void wg_clear(gfx_rect_t r)
{
    gfx_fill(r, WG_COLOR_BG);
}

void wg_label(gfx_rect_t r, const char *text, gfx_align_t align, uint16_t color, uint16_t bg)
{
    gfx_text_in(r, text, align, color, bg, 0);
}

void wg_clock(gfx_rect_t r, uint32_t seconds, uint16_t bg)
{
    /* ЧЧ:ММ:СС, после 99 часов счёт идёт заново */
    uint32_t h = (seconds / 3600) % 100;
    uint32_t m = (seconds / 60) % 60;
    uint32_t s = seconds % 60;
    char text[9] = {
        '0' + h / 10, '0' + h % 10, ':',
        '0' + m / 10, '0' + m % 10, ':',
        '0' + s / 10, '0' + s % 10, '\0',
    };

    gfx_text_in(r, text, GFX_ALIGN_LEFT, WG_COLOR_TEXT, bg, 0);
}

void wg_status_bar(gfx_rect_t r)
{
    gfx_frame(r, WG_COLOR_PANEL, WG_COLOR_BORDER);
}

void wg_vehicle_panel(gfx_rect_t r, const char *name, const char *summary, bool selected)
{
    uint16_t fill = selected ? WG_COLOR_SELECTED : WG_COLOR_PANEL;

    /* Название крупно сверху, краткая телеметрия под ним. Вместе закрывают всю панель */
    gfx_rect_t top = {r.x + 1, r.y + 1, r.w - 2, r.h / 2 + 4};
    gfx_rect_t bottom = {r.x + 1, top.y + top.h, r.w - 2, r.h - 2 - top.h};

    gfx_border(r, WG_COLOR_BORDER);
    gfx_text_in(top, name, GFX_ALIGN_CENTER, WG_COLOR_TEXT, fill, 1);
    gfx_text_in(bottom, summary, GFX_ALIGN_CENTER, WG_COLOR_TEXT, fill, 0);
}

void wg_tab(gfx_rect_t r, bool up)
{
    uint16_t left = r.x + 6;
    uint16_t right = r.x + r.w - 7;
    uint16_t mid = r.x + r.w / 2;
    uint16_t top = r.y + 3;
    uint16_t bottom = r.y + r.h - 4;

    gfx_frame(r, WG_COLOR_PANEL, WG_COLOR_BORDER);

    /* Пологая галочка во всю ширину: куда смотрит, туда и уедет панель */
    if (up)
    {
        gfx_line(left, bottom, mid, top, WG_COLOR_BORDER);
        gfx_line(mid, top, right, bottom, WG_COLOR_BORDER);
    }
    else
    {
        gfx_line(left, top, mid, bottom, WG_COLOR_BORDER);
        gfx_line(mid, bottom, right, top, WG_COLOR_BORDER);
    }
}

static const char *next_line(const char *text)
{
    while (text != NULL && *text != '\0')
    {
        if (*text++ == '\n')
        {
            return text;
        }
    }
    return NULL;
}

void wg_text_box(gfx_rect_t r, const char *text)
{
    gfx_fill(r, WG_COLOR_BG);
    gfx_dashed_rect(r, WG_COLOR_BORDER);
    wg_text_lines(r, text);
}

void wg_text_lines(gfx_rect_t r, const char *text)
{
    gfx_rect_t row = {r.x + PAD, r.y + PAD, r.w - 2 * PAD, LINE_H};
    uint16_t bottom = r.y + r.h - PAD;

    /* Каждая строка сама заливает свой фон, общей очистки нет, поэтому не мерцает */
    while (row.y + LINE_H <= bottom)
    {
        gfx_text_in(row, text, GFX_ALIGN_LEFT, WG_COLOR_TEXT, WG_COLOR_BG, 0);
        text = next_line(text);
        row.y += LINE_H;
    }

    if (row.y < bottom)
    {
        row.h = bottom - row.y;
        gfx_fill(row, WG_COLOR_BG);
    }
}

void wg_assign_list(gfx_rect_t r, gfx_side_t side,
                    const char *const buttons[WG_BUTTONS], const char *stick)
{
    bool left = side == GFX_SIDE_LEFT;
    gfx_align_t align = left ? GFX_ALIGN_LEFT : GFX_ALIGN_RIGHT;
    uint16_t arrow_x = left ? r.x + PAD + ARROW_SIZE / 2 : r.x + r.w - PAD - ARROW_SIZE / 2 - 1;
    uint16_t text_x = left ? r.x + PAD + ARROW_SIZE + ARROW_GAP : r.x + PAD;
    uint16_t text_w = r.w - 2 * PAD - ARROW_SIZE - ARROW_GAP;

    gfx_fill(r, WG_COLOR_BG);
    gfx_dashed_rect(r, WG_COLOR_BORDER);

    /* Назначение стика сверху: так его рисуют, когда экранных стиков нет */
    if (stick != NULL)
    {
        gfx_rect_t line = {r.x + PAD, r.y + PAD, r.w - 2 * PAD, ASSIGN_H};
        gfx_text_in(line, stick, align, WG_COLOR_TEXT, WG_COLOR_BG, 0);
    }

    /* Кнопки прижаты к низу рамки, стрелки со стороны крестовины */
    uint16_t y = r.y + r.h - PAD - WG_BUTTONS * ASSIGN_H;

    for (uint8_t i = 0; i < WG_BUTTONS; i++, y += ASSIGN_H)
    {
        gfx_rect_t text = {text_x, y, text_w, ASSIGN_H};

        gfx_arrow(arrow_x, y + ASSIGN_H / 2, ARROW_SIZE, (gfx_dir_t)i, WG_COLOR_TEXT);
        gfx_text_in(text, buttons[i], align, WG_COLOR_TEXT, WG_COLOR_BG, 0);
    }
}

void wg_round_button(uint16_t cx, uint16_t cy, uint8_t radius, gfx_dir_t dir, bool pressed)
{
    gfx_circle(cx, cy, radius, pressed ? WG_COLOR_PRESSED : WG_COLOR_PANEL, WG_COLOR_BORDER);
    gfx_arrow(cx, cy, radius + radius / 4, dir, WG_COLOR_BORDER);
}

void wg_stick(uint16_t cx, uint16_t cy, uint8_t radius, bool pressed)
{
    gfx_circle(cx, cy, radius, pressed ? WG_COLOR_PRESSED : WG_COLOR_PANEL, WG_COLOR_BORDER);
}

void wg_logo(uint16_t cx, uint16_t cy, uint8_t radius)
{
    gfx_logo(cx, cy, radius);
}

bool wg_icon_visible(wg_icon_t state, bool blink_on)
{
    return state == WG_ICON_ON || (state == WG_ICON_BLINK && blink_on);
}

void wg_joycons(uint16_t x, uint16_t y, bool left, bool right, uint16_t bg)
{
    uint16_t right_x = x + GFX_JOYCON_W + JOYCON_GAP;

    if (left)
    {
        gfx_icon_joycon(x, y, GFX_SIDE_LEFT, WG_COLOR_TEXT, bg);
    }
    else
    {
        gfx_fill((gfx_rect_t){x, y, GFX_JOYCON_W, GFX_JOYCON_H}, bg);
    }

    if (right)
    {
        gfx_icon_joycon(right_x, y, GFX_SIDE_RIGHT, WG_COLOR_TEXT, bg);
    }
    else
    {
        gfx_fill((gfx_rect_t){right_x, y, GFX_JOYCON_W, GFX_JOYCON_H}, bg);
    }
}

void wg_bluetooth(uint16_t x, uint16_t y, bool visible, uint16_t bg)
{
    gfx_fill((gfx_rect_t){x, y, GFX_BLUETOOTH_W, GFX_BLUETOOTH_H}, bg);
    if (visible)
    {
        gfx_icon_bluetooth(x, y, WG_COLOR_TEXT);
    }
}

void wg_battery(uint16_t x, uint16_t y, uint8_t level, uint16_t bg)
{
    /* Последнее деление и пустая батарея - красным */
    gfx_icon_battery(x, y, level, level <= 1 ? WG_COLOR_ALERT : WG_COLOR_TEXT, bg);
}

bool wg_hit(gfx_rect_t r, uint16_t x, uint16_t y)
{
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

bool wg_hit_circle(uint16_t cx, uint16_t cy, uint8_t radius, uint16_t x, uint16_t y)
{
    int32_t dx = (int32_t)x - cx;
    int32_t dy = (int32_t)y - cy;

    return dx * dx + dy * dy <= (int32_t)radius * radius;
}
