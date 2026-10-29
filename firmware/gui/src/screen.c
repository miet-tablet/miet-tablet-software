#include "screen.h"

/* Размеры сняты с эскиза 154,88 x 86,72 мм и пересчитаны в 800 x 480 px */
#define STATUS_H          30   /* статус-бар, 5 мм */
#define PANELS_Y          (STATUS_H - 1)  /* общая граница со статус-баром */
#define PANELS_H          82   /* панели изделий до 20 мм от верха */
#define TAB_W             258
#define TAB_H             14
#define TAB_X             ((GFX_WIDTH - TAB_W) / 2)
#define TAB_Y             116  /* 21 мм */
#define TAB_Y_UP          38   /* при скрытых панелях: в строке с названием */
#define TAB_HIT           10   /* язычок узкий: зона касания шире сверху и снизу */
#define NAME_H            30   /* строка с названием при скрытых панелях */
#define AREA_Y            136  /* верх рамок под панелями */
#define AREA_Y_UP         66   /* верх рамок при скрытых панелях */
#define AREA_BOTTOM       470  /* низ рамок при физических джойстиках */
#define TELEMETRY_BOTTOM  293  /* низ телеметрии над экранным управлением */
#define SIDE_X            25   /* 5 мм от края */
#define SIDE_X_JOY        10   /* при джойстиках рамки уходят к краям */
#define SIDE_W            220
#define SIDE_H            124
#define GAP               10   /* между рамками */
#define STICK_LABEL_Y     300
#define STICK_LABEL_W     200
#define STICK_R           62   /* R12 мм */
#define STICK_X           72   /* 14 мм от края */
#define STICK_Y           403
#define DPAD_X            227  /* 30 мм от центра стика */
#define DPAD_Y            391
#define DPAD_DX           51   /* крестовина 20 x 16 мм */
#define DPAD_DY           44
#define BUTTON_R          21   /* R4 мм */
#define LOGO_Y            430
#define LOGO_R            30

/* Значки статус-бара, справа налево: батарея, Bluetooth, джойконы */
#define ICON_Y            5
#define BATTERY_X         (GFX_WIDTH - 10 - GFX_BATTERY_W)
#define BLUETOOTH_X       (BATTERY_X - 16 - GFX_BLUETOOTH_W)
#define JOYCON_X          (BLUETOOTH_X - 14 - WG_JOYCONS_W)

/* Сколько элементов касания на сторону: 4 кнопки и стик */
#define SIDE_HITS         (WG_BUTTONS + 1)

static const gfx_rect_t status_rect = {0, 0, GFX_WIDTH, STATUS_H};
static const gfx_rect_t clock_rect = {8, 1, 80, STATUS_H - 2};
static const gfx_rect_t message_rect = {100, 1, JOYCON_X - 110, STATUS_H - 2};
static const gfx_rect_t name_rect = {8, STATUS_H, TAB_X - 16, NAME_H};

typedef struct
{
    gfx_rect_t tab;
    gfx_rect_t side[2];  /* рамки назначений: левая, правая */
    gfx_rect_t center;   /* подробная телеметрия */
} layout_t;

static layout_t make_layout(const screen_model_t *m)
{
    layout_t l;
    uint16_t top = m->panels_visible ? AREA_Y : AREA_Y_UP;

    l.tab = (gfx_rect_t){TAB_X, m->panels_visible ? TAB_Y : TAB_Y_UP, TAB_W, TAB_H};

    if (m->joystick_mode)
    {
        /* Рамки до низа экрана: описания кнопок напротив кнопок джойстиков */
        uint16_t center_x = SIDE_X_JOY + SIDE_W + GAP;

        l.side[0] = (gfx_rect_t){SIDE_X_JOY, top, SIDE_W, AREA_BOTTOM - top};
        l.side[1] = (gfx_rect_t){GFX_WIDTH - SIDE_X_JOY - SIDE_W, top, SIDE_W, AREA_BOTTOM - top};
        l.center = (gfx_rect_t){center_x, top, GFX_WIDTH - 2 * center_x, AREA_BOTTOM - top};
    }
    else
    {
        /* Рамки назначений на месте, место от скрытых панелей забирает телеметрия */
        uint16_t center_x = SIDE_X + SIDE_W + GAP;

        l.side[0] = (gfx_rect_t){SIDE_X, AREA_Y, SIDE_W, SIDE_H};
        l.side[1] = (gfx_rect_t){GFX_WIDTH - SIDE_X - SIDE_W, AREA_Y, SIDE_W, SIDE_H};
        l.center = (gfx_rect_t){center_x, top, GFX_WIDTH - 2 * center_x, TELEMETRY_BOTTOM - top};
    }

    return l;
}

/* Соседние панели делят одну вертикальную границу */
static gfx_rect_t panel_rect(uint8_t i)
{
    uint16_t x0 = i * GFX_WIDTH / SCREEN_VEHICLES;
    uint16_t x1 = (i + 1) * GFX_WIDTH / SCREEN_VEHICLES;

    return (gfx_rect_t){x0, PANELS_Y, x1 - x0 + (i + 1 < SCREEN_VEHICLES ? 1 : 0), PANELS_H};
}

static gfx_rect_t stick_label_rect(uint8_t side)
{
    uint16_t x = side == 0 ? SIDE_X : GFX_WIDTH - SIDE_X - STICK_LABEL_W;

    return (gfx_rect_t){x, STICK_LABEL_Y, STICK_LABEL_W, GFX_CHAR_H};
}

/* side: 0 - левая, 1 - правая; i: вверх, влево, вправо, вниз */
static void button_center(uint8_t side, uint8_t i, uint16_t *x, uint16_t *y)
{
    static const int8_t dx[WG_BUTTONS] = {0, -1, 1, 0};
    static const int8_t dy[WG_BUTTONS] = {-1, 0, 0, 1};
    uint16_t cx = side == 0 ? DPAD_X : GFX_WIDTH - DPAD_X;

    *x = cx + dx[i] * DPAD_DX;
    *y = DPAD_Y + dy[i] * DPAD_DY;
}

static uint16_t stick_x(uint8_t side)
{
    return side == 0 ? STICK_X : GFX_WIDTH - STICK_X;
}

static screen_hit_t side_hit(uint8_t side, uint8_t i)
{
    return (screen_hit_t)(SCREEN_HIT_LEFT_UP + side * SIDE_HITS + i);
}

void screen_draw_all(const screen_model_t *m)
{
    wg_clear((gfx_rect_t){0, 0, GFX_WIDTH, GFX_HEIGHT});
    wg_status_bar(status_rect);
    screen_draw_clock(m);
    screen_draw_message(m);
    screen_draw_icons(m);
    screen_draw_battery(m);
    screen_draw_panels(m);
    wg_tab(make_layout(m).tab, m->panels_visible);
    screen_draw_vehicle(m);
    screen_draw_controls(m);
}

void screen_draw_clock(const screen_model_t *m)
{
    wg_clock(clock_rect, m->seconds, WG_COLOR_PANEL);
}

void screen_draw_message(const screen_model_t *m)
{
    wg_label(message_rect, m->message, GFX_ALIGN_CENTER, WG_COLOR_ALERT, WG_COLOR_PANEL);
}

void screen_draw_icons(const screen_model_t *m)
{
    wg_joycons(JOYCON_X, ICON_Y,
               wg_icon_visible(m->joycon[0], m->blink_on),
               wg_icon_visible(m->joycon[1], m->blink_on), WG_COLOR_PANEL);
    wg_bluetooth(BLUETOOTH_X, ICON_Y, wg_icon_visible(m->bluetooth, m->blink_on), WG_COLOR_PANEL);
}

void screen_draw_battery(const screen_model_t *m)
{
    wg_battery(BATTERY_X, ICON_Y + 1, m->battery, WG_COLOR_PANEL);
}

void screen_draw_panels(const screen_model_t *m)
{
    if (!m->panels_visible)
    {
        /* Панели скрыты: слева остаётся название выбранного изделия */
        wg_label(name_rect, m->vehicles[m->selected].name, GFX_ALIGN_LEFT, WG_COLOR_TEXT, WG_COLOR_BG);
        return;
    }

    for (uint8_t i = 0; i < SCREEN_VEHICLES; i++)
    {
        wg_vehicle_panel(panel_rect(i), m->vehicles[i].name, m->summary[i], i == m->selected);
    }
}

void screen_draw_vehicle(const screen_model_t *m)
{
    layout_t l = make_layout(m);
    const screen_vehicle_t *v = &m->vehicles[m->selected];

    for (uint8_t s = 0; s < 2; s++)
    {
        /* Без экранных стиков их назначение пишется в рамке, иначе - над стиком */
        wg_assign_list(l.side[s], s == 0 ? GFX_SIDE_LEFT : GFX_SIDE_RIGHT, v->buttons[s],
                       m->joystick_mode ? v->sticks[s] : NULL);
    }
    wg_text_box(l.center, m->details[m->selected]);

    if (!m->joystick_mode)
    {
        wg_label(stick_label_rect(0), v->sticks[0], GFX_ALIGN_LEFT, WG_COLOR_TEXT, WG_COLOR_BG);
        wg_label(stick_label_rect(1), v->sticks[1], GFX_ALIGN_RIGHT, WG_COLOR_TEXT, WG_COLOR_BG);
    }
}

void screen_draw_telemetry(const screen_model_t *m)
{
    wg_text_lines(make_layout(m).center, m->details[m->selected]);
}

void screen_draw_controls(const screen_model_t *m)
{
    if (m->joystick_mode)
    {
        return;
    }

    for (uint8_t s = 0; s < 2; s++)
    {
        for (uint8_t i = 0; i < WG_BUTTONS; i++)
        {
            uint16_t x;
            uint16_t y;

            button_center(s, i, &x, &y);
            wg_round_button(x, y, BUTTON_R, (gfx_dir_t)i, m->pressed == side_hit(s, i));
        }
        wg_stick(stick_x(s), STICK_Y, STICK_R, m->pressed == side_hit(s, WG_BUTTONS));
    }

    wg_logo(GFX_WIDTH / 2, LOGO_Y, LOGO_R);
}

screen_hit_t screen_hit(const screen_model_t *m, uint16_t x, uint16_t y)
{
    layout_t l = make_layout(m);
    gfx_rect_t tab_zone = {l.tab.x, l.tab.y - TAB_HIT, l.tab.w, l.tab.h + 2 * TAB_HIT};

    if (x >= GFX_WIDTH || y >= GFX_HEIGHT)
    {
        return SCREEN_HIT_NONE;
    }

    if (m->panels_visible && y >= PANELS_Y && y < PANELS_Y + PANELS_H)
    {
        return (screen_hit_t)(SCREEN_HIT_VEHICLE_0 + x * SCREEN_VEHICLES / GFX_WIDTH);
    }

    if (wg_hit(tab_zone, x, y))
    {
        return SCREEN_HIT_TAB;
    }

    if (m->joystick_mode)
    {
        return SCREEN_HIT_NONE;
    }

    for (uint8_t s = 0; s < 2; s++)
    {
        for (uint8_t i = 0; i < WG_BUTTONS; i++)
        {
            uint16_t bx;
            uint16_t by;

            button_center(s, i, &bx, &by);
            if (wg_hit_circle(bx, by, BUTTON_R, x, y))
            {
                return side_hit(s, i);
            }
        }
        if (wg_hit_circle(stick_x(s), STICK_Y, STICK_R, x, y))
        {
            return side_hit(s, WG_BUTTONS);
        }
    }

    return SCREEN_HIT_NONE;
}
