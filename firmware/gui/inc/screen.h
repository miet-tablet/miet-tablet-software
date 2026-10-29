#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "widgets.h"

/*
 * Уровень 3: раскладка экрана. Расставляет виджеты по эскизу и рисует
 * зоны экрана по модели. Что показывать, решает уровень 4 (gui).
 */

#define SCREEN_VEHICLES  3

/* Изделие: название и назначения органов управления */
typedef struct
{
    const char *name;
    const char *buttons[2][WG_BUTTONS];  /* [левая, правая крестовина][вверх, влево, вправо, вниз] */
    const char *sticks[2];               /* [левый, правый стик] */
} screen_vehicle_t;

/* Куда попало касание */
typedef enum
{
    SCREEN_HIT_NONE,
    SCREEN_HIT_VEHICLE_0,     /* панели изделий слева направо */
    SCREEN_HIT_VEHICLE_1,
    SCREEN_HIT_VEHICLE_2,
    SCREEN_HIT_TAB,           /* язычок: скрыть или показать панели */
    SCREEN_HIT_LEFT_UP,       /* левая крестовина и стик */
    SCREEN_HIT_LEFT_LEFT,
    SCREEN_HIT_LEFT_RIGHT,
    SCREEN_HIT_LEFT_DOWN,
    SCREEN_HIT_LEFT_STICK,
    SCREEN_HIT_RIGHT_UP,      /* правая крестовина и стик */
    SCREEN_HIT_RIGHT_LEFT,
    SCREEN_HIT_RIGHT_RIGHT,
    SCREEN_HIT_RIGHT_DOWN,
    SCREEN_HIT_RIGHT_STICK,
} screen_hit_t;

/* Модель: всё, что сейчас должно быть на экране */
typedef struct
{
    const screen_vehicle_t *vehicles;      /* SCREEN_VEHICLES штук */
    const char *summary[SCREEN_VEHICLES];  /* краткая телеметрия на панелях */
    const char *details[SCREEN_VEHICLES];  /* подробная, строки через '\n' */
    const char *message;                   /* сообщение об ошибке в статус-баре */
    uint8_t selected;                      /* выбранное изделие */
    bool panels_visible;                   /* ряд панелей изделий не скрыт */
    bool joystick_mode;                    /* физические джойстики: экранного управления нет */
    wg_icon_t joycon[2];                   /* значки левого и правого джойкона */
    wg_icon_t bluetooth;
    uint8_t battery;                       /* делений 0..5 */
    bool blink_on;                         /* фаза мигания значков */
    uint32_t seconds;                      /* для часов */
    screen_hit_t pressed;                  /* удерживаемая экранная кнопка или стик */
} screen_model_t;

/* Весь экран целиком */
void screen_draw_all(const screen_model_t *m);

/* Отдельные зоны, чтобы не перерисовывать лишнее */
void screen_draw_clock(const screen_model_t *m);
void screen_draw_message(const screen_model_t *m);
void screen_draw_icons(const screen_model_t *m);      /* джойконы и Bluetooth */
void screen_draw_battery(const screen_model_t *m);
void screen_draw_panels(const screen_model_t *m);     /* панели или название при скрытых */
void screen_draw_vehicle(const screen_model_t *m);    /* назначения и телеметрия выбранного */
void screen_draw_telemetry(const screen_model_t *m);  /* только текст телеметрии */
void screen_draw_controls(const screen_model_t *m);   /* экранные кнопки и стики */

screen_hit_t screen_hit(const screen_model_t *m, uint16_t x, uint16_t y);

#if defined(__cplusplus)
}
#endif
