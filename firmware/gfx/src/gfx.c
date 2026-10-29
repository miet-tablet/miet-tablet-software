#include "gfx.h"

#define DASH_LEN  6
#define DASH_GAP  4
#define TEXT_MAX  (GFX_WIDTH / GFX_CHAR_W)

int gfx_init(void)
{
    return ra8875_init();
}

void gfx_clear(uint16_t color)
{
    ra8875_fill_screen(color);
}

void gfx_fill(gfx_rect_t r, uint16_t color)
{
    ra8875_fill_rect(r.x, r.y, r.w, r.h, color);
}

void gfx_border(gfx_rect_t r, uint16_t color)
{
    ra8875_draw_rect(r.x, r.y, r.w, r.h, color);
}

void gfx_frame(gfx_rect_t r, uint16_t fill, uint16_t border)
{
    ra8875_fill_rect(r.x, r.y, r.w, r.h, fill);
    ra8875_draw_rect(r.x, r.y, r.w, r.h, border);
}

static void dashed_hline(uint16_t x0, uint16_t x1, uint16_t y, uint16_t color)
{
    for (uint16_t x = x0; x <= x1; x += DASH_LEN + DASH_GAP)
    {
        uint16_t end = x + DASH_LEN - 1;
        ra8875_draw_line(x, y, end < x1 ? end : x1, y, color);
    }
}

static void dashed_vline(uint16_t x, uint16_t y0, uint16_t y1, uint16_t color)
{
    for (uint16_t y = y0; y <= y1; y += DASH_LEN + DASH_GAP)
    {
        uint16_t end = y + DASH_LEN - 1;
        ra8875_draw_line(x, y, x, end < y1 ? end : y1, color);
    }
}

void gfx_dashed_rect(gfx_rect_t r, uint16_t color)
{
    if (r.w == 0 || r.h == 0)
    {
        return;
    }

    uint16_t x1 = r.x + r.w - 1;
    uint16_t y1 = r.y + r.h - 1;

    dashed_hline(r.x, x1, r.y, color);
    dashed_hline(r.x, x1, y1, color);
    dashed_vline(r.x, r.y, y1, color);
    dashed_vline(x1, r.y, y1, color);
}

void gfx_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    ra8875_draw_line(x0, y0, x1, y1, color);
}

void gfx_circle(uint16_t cx, uint16_t cy, uint8_t r, uint16_t fill, uint16_t border)
{
    ra8875_fill_circle(cx, cy, r, fill);
    ra8875_draw_circle(cx, cy, r, border);
}

static uint16_t text_len(const char *text)
{
    uint16_t n = 0;

    if (text == NULL)
    {
        return 0;
    }
    while (text[n] != '\0' && text[n] != '\n')
    {
        n++;
    }

    return n;
}

uint16_t gfx_text_width(const char *text, uint8_t scale)
{
    if (scale > 3)
    {
        scale = 3;
    }
    return text_len(text) * GFX_CHAR_W * (scale + 1);
}

void gfx_text_in(gfx_rect_t r, const char *text, gfx_align_t align,
                 uint16_t fg, uint16_t bg, uint8_t scale)
{
    char buf[TEXT_MAX + 1];

    if (scale > 3)
    {
        scale = 3;
    }

    uint16_t cw = GFX_CHAR_W * (scale + 1);
    uint16_t ch = GFX_CHAR_H * (scale + 1);
    uint16_t n = text_len(text);

    /* Лишнее обрезаем, иначе контроллер перенесёт текст на новую строку */
    if (n > r.w / cw)
    {
        n = r.w / cw;
    }
    if (n > TEXT_MAX)
    {
        n = TEXT_MAX;
    }
    if (n == 0 || r.h < ch)
    {
        gfx_fill(r, bg);
        return;
    }

    for (uint16_t i = 0; i < n; i++)
    {
        buf[i] = text[i];
    }
    buf[n] = '\0';

    uint16_t tw = n * cw;
    uint16_t tx = r.x;
    uint16_t ty = r.y + (r.h - ch) / 2;

    if (align == GFX_ALIGN_CENTER)
    {
        tx = r.x + (r.w - tw) / 2;
    }
    else if (align == GFX_ALIGN_RIGHT)
    {
        tx = r.x + r.w - tw;
    }

    /* Фон вокруг текста: слева, справа, сверху, снизу. Под текстом его рисует контроллер */
    ra8875_fill_rect(r.x, r.y, tx - r.x, r.h, bg);
    ra8875_fill_rect(tx + tw, r.y, r.x + r.w - tx - tw, r.h, bg);
    ra8875_fill_rect(tx, r.y, tw, ty - r.y, bg);
    ra8875_fill_rect(tx, ty + ch, tw, r.y + r.h - ty - ch, bg);

    ra8875_draw_text(tx, ty, buf, fg, bg, scale);
}

void gfx_arrow(uint16_t cx, uint16_t cy, uint16_t size, gfx_dir_t dir, uint16_t color)
{
    int16_t h = size / 2;

    /* Галочка стрелки вверх: конец, вершина, конец. Остальные - её поворот */
    const int16_t up_x[3] = {-h, 0, h};
    const int16_t up_y[3] = {h, -h, h};
    int16_t x[3];
    int16_t y[3];

    for (int i = 0; i < 3; i++)
    {
        switch (dir)
        {
        case GFX_DIR_LEFT:
            x[i] = up_y[i];
            y[i] = up_x[i];
            break;
        case GFX_DIR_RIGHT:
            x[i] = -up_y[i];
            y[i] = up_x[i];
            break;
        case GFX_DIR_DOWN:
            x[i] = up_x[i];
            y[i] = -up_y[i];
            break;
        default:
            x[i] = up_x[i];
            y[i] = up_y[i];
            break;
        }
    }

    /* Толщина 2 px: второй проход со сдвигом поперёк стрелки */
    int16_t dx = (dir == GFX_DIR_UP || dir == GFX_DIR_DOWN) ? 1 : 0;
    int16_t dy = 1 - dx;

    for (int k = 0; k < 2; k++)
    {
        for (int i = 0; i < 2; i++)
        {
            ra8875_draw_line(cx + x[i] + k * dx, cy + y[i] + k * dy,
                             cx + x[i + 1] + k * dx, cy + y[i + 1] + k * dy, color);
        }
    }
}

void gfx_icon_battery(uint16_t x, uint16_t y, uint8_t level, uint16_t color, uint16_t bg)
{
    /* Корпус 32 x 18 с рамкой 2 px, контакт справа, внутри level делений из 5 */
    if (level > 5)
    {
        level = 5;
    }

    ra8875_fill_rect(x + 2, y + 2, 28, GFX_BATTERY_H - 4, bg);
    ra8875_draw_rect(x, y, 32, GFX_BATTERY_H, color);
    ra8875_draw_rect(x + 1, y + 1, 30, GFX_BATTERY_H - 2, color);
    ra8875_fill_rect(x + 32, y + 5, GFX_BATTERY_W - 32, 8, color);

    for (uint8_t i = 0; i < level; i++)
    {
        ra8875_fill_rect(x + 4 + i * 5, y + 4, 4, GFX_BATTERY_H - 8, color);
    }
}

void gfx_icon_bluetooth(uint16_t x, uint16_t y, uint16_t color)
{
    /* Руна Bluetooth: ствол и две петли, линии 2 px */
    for (uint16_t k = 0; k < 2; k++)
    {
        uint16_t s = x + 5 + k;

        ra8875_draw_line(s - 5, y + 5, s + 5, y + 14, color);
        ra8875_draw_line(s + 5, y + 14, s, y + 19, color);
        ra8875_draw_line(s, y + 19, s, y, color);
        ra8875_draw_line(s, y, s + 5, y + 5, color);
        ra8875_draw_line(s + 5, y + 5, s - 5, y + 14, color);
    }
}

void gfx_icon_joycon(uint16_t x, uint16_t y, gfx_side_t side, uint16_t color, uint16_t bg)
{
    /* Половинка джойкона: корпус, стик и кнопка. У правой они меняются местами */
    uint16_t stick_y = side == GFX_SIDE_LEFT ? y + 6 : y + 13;
    uint16_t button_y = side == GFX_SIDE_LEFT ? y + 13 : y + 6;

    ra8875_fill_rect(x, y, GFX_JOYCON_W, GFX_JOYCON_H, bg);
    ra8875_draw_rect(x, y, GFX_JOYCON_W, GFX_JOYCON_H, color);
    ra8875_draw_rect(x + 1, y + 1, GFX_JOYCON_W - 2, GFX_JOYCON_H - 2, color);
    ra8875_fill_circle(x + 5, stick_y, 2, color);
    ra8875_fill_rect(x + 4, button_y - 1, 3, 3, color);
}

void gfx_logo(uint16_t cx, uint16_t cy, uint8_t r)
{
    gfx_rect_t text = {cx - 2 * GFX_CHAR_W, cy - GFX_CHAR_H / 2, 4 * GFX_CHAR_W, GFX_CHAR_H};

    ra8875_fill_circle(cx, cy, r, GFX_BLACK);
    ra8875_draw_circle(cx, cy, r - 4, GFX_WHITE);
    gfx_text_in(text, "MIET", GFX_ALIGN_CENTER, GFX_WHITE, GFX_BLACK, 0);
}
