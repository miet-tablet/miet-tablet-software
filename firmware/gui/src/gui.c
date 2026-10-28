#include "gui.h"

#define BLINK_MS  500

/* Что перерисовать в ближайшем gui_tick */
#define DIRTY_ALL        (1U << 0)
#define DIRTY_CLOCK      (1U << 1)
#define DIRTY_ICONS      (1U << 2)
#define DIRTY_BATTERY    (1U << 3)
#define DIRTY_MESSAGE    (1U << 4)
#define DIRTY_PANELS     (1U << 5)
#define DIRTY_VEHICLE    (1U << 6)
#define DIRTY_TELEMETRY  (1U << 7)
#define DIRTY_CONTROLS   (1U << 8)

static screen_model_t model;
static char summaries[GUI_VEHICLES][GUI_SUMMARY_LEN];
static char details[GUI_VEHICLES][GUI_DETAILS_LEN];
static char message[GUI_MESSAGE_LEN];
static uint32_t dirty;
static uint32_t blink_ms;
static bool touch_down;

/* Копирует src в dst с обрезкой. true - текст изменился */
static bool set_text(char *dst, uint32_t size, const char *src)
{
    bool changed = false;
    uint32_t i = 0;

    if (src != NULL)
    {
        for (; i + 1 < size && src[i] != '\0'; i++)
        {
            if (dst[i] != src[i])
            {
                dst[i] = src[i];
                changed = true;
            }
        }
    }

    if (dst[i] != '\0')
    {
        dst[i] = '\0';
        changed = true;
    }

    return changed;
}

int gui_init(const gui_vehicle_t *vehicles)
{
    if (gfx_init() != 0)
    {
        return -1;
    }

    model.vehicles = vehicles;
    for (uint8_t i = 0; i < GUI_VEHICLES; i++)
    {
        model.summary[i] = summaries[i];
        model.details[i] = details[i];
    }
    model.message = message;
    model.selected = 0;
    model.panels_visible = true;
    model.pressed = SCREEN_HIT_NONE;

    dirty = DIRTY_ALL;
    return 0;
}

void gui_set_battery(uint8_t level)
{
    if (level > 5)
    {
        level = 5;
    }
    if (level != model.battery)
    {
        model.battery = level;
        dirty |= DIRTY_BATTERY;
    }
}

void gui_set_bluetooth(gui_bt_t state)
{
    static const wg_icon_t icon[] = {WG_ICON_OFF, WG_ICON_BLINK, WG_ICON_ON};

    if (state > GUI_BT_CONNECTED || icon[state] == model.bluetooth)
    {
        return;
    }
    model.bluetooth = icon[state];
    dirty |= DIRTY_ICONS;
}

void gui_set_joysticks(bool left, bool right)
{
    /* Подключён один: он горит, второй мигает. Оба: горят, экранное управление убирается */
    wg_icon_t l = left ? WG_ICON_ON : (right ? WG_ICON_BLINK : WG_ICON_OFF);
    wg_icon_t r = right ? WG_ICON_ON : (left ? WG_ICON_BLINK : WG_ICON_OFF);
    bool joystick_mode = left && right;

    if (l != model.joycon[0] || r != model.joycon[1])
    {
        model.joycon[0] = l;
        model.joycon[1] = r;
        dirty |= DIRTY_ICONS;
    }

    if (joystick_mode != model.joystick_mode)
    {
        model.joystick_mode = joystick_mode;
        model.pressed = SCREEN_HIT_NONE;
        dirty |= DIRTY_ALL;
    }
}

void gui_set_telemetry(uint8_t vehicle, const char *summary, const char *text)
{
    if (vehicle >= GUI_VEHICLES)
    {
        return;
    }

    if (set_text(summaries[vehicle], GUI_SUMMARY_LEN, summary) && model.panels_visible)
    {
        dirty |= DIRTY_PANELS;
    }
    if (set_text(details[vehicle], GUI_DETAILS_LEN, text) && vehicle == model.selected)
    {
        dirty |= DIRTY_TELEMETRY;
    }
}

void gui_set_message(const char *text)
{
    if (set_text(message, GUI_MESSAGE_LEN, text))
    {
        dirty |= DIRTY_MESSAGE;
    }
}

void gui_select(uint8_t vehicle)
{
    if (vehicle < GUI_VEHICLES && vehicle != model.selected)
    {
        model.selected = vehicle;
        dirty |= DIRTY_PANELS | DIRTY_VEHICLE;
    }
}

void gui_toggle_panels(void)
{
    model.panels_visible = !model.panels_visible;
    dirty |= DIRTY_ALL;
}

screen_hit_t gui_touch(uint16_t x, uint16_t y, bool pressed)
{
    screen_hit_t hit = pressed ? screen_hit(&model, x, y) : SCREEN_HIT_NONE;

    /* Панели и язычок срабатывают один раз, в момент касания */
    if (pressed && !touch_down)
    {
        if (hit >= SCREEN_HIT_VEHICLE_0 && hit <= SCREEN_HIT_VEHICLE_2)
        {
            gui_select(hit - SCREEN_HIT_VEHICLE_0);
        }
        else if (hit == SCREEN_HIT_TAB)
        {
            gui_toggle_panels();
        }
    }
    touch_down = pressed;

    /* Экранные кнопки и стики подсвечиваются, пока их держат */
    screen_hit_t control = hit >= SCREEN_HIT_LEFT_UP ? hit : SCREEN_HIT_NONE;

    if (control != model.pressed)
    {
        model.pressed = control;
        dirty |= DIRTY_CONTROLS;
    }

    return hit;
}

screen_hit_t gui_pressed(void)
{
    return model.pressed;
}

void gui_tick(uint32_t now_ms)
{
    uint32_t seconds = now_ms / 1000;

    if (seconds != model.seconds)
    {
        model.seconds = seconds;
        dirty |= DIRTY_CLOCK;
    }

    if (now_ms - blink_ms >= BLINK_MS)
    {
        blink_ms = now_ms;
        model.blink_on = !model.blink_on;
        if (model.joycon[0] == WG_ICON_BLINK || model.joycon[1] == WG_ICON_BLINK ||
            model.bluetooth == WG_ICON_BLINK)
        {
            dirty |= DIRTY_ICONS;
        }
    }

    if (dirty & DIRTY_ALL)
    {
        screen_draw_all(&model);
        dirty = 0;
        return;
    }

    if (dirty & DIRTY_CLOCK)
    {
        screen_draw_clock(&model);
    }
    if (dirty & DIRTY_ICONS)
    {
        screen_draw_icons(&model);
    }
    if (dirty & DIRTY_BATTERY)
    {
        screen_draw_battery(&model);
    }
    if (dirty & DIRTY_MESSAGE)
    {
        screen_draw_message(&model);
    }
    if (dirty & DIRTY_PANELS)
    {
        screen_draw_panels(&model);
    }
    if (dirty & DIRTY_VEHICLE)
    {
        screen_draw_vehicle(&model);
    }
    else if (dirty & DIRTY_TELEMETRY)
    {
        screen_draw_telemetry(&model);
    }
    if (dirty & DIRTY_CONTROLS)
    {
        screen_draw_controls(&model);
    }

    dirty = 0;
}
