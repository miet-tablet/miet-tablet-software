#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "screen.h"

/*
 * Уровень 4: состояние и логика интерфейса. Снаружи используется только он.
 * Сеттеры только запоминают данные, рисует gui_tick, и только изменившееся.
 */

#define GUI_WIDTH        GFX_WIDTH
#define GUI_HEIGHT       GFX_HEIGHT
#define GUI_VEHICLES     SCREEN_VEHICLES
#define GUI_SUMMARY_LEN  40   /* с '\0' */
#define GUI_DETAILS_LEN  400
#define GUI_MESSAGE_LEN  72

typedef screen_vehicle_t gui_vehicle_t;

typedef enum
{
    GUI_BT_OFF,        /* нет соединения: значка нет */
    GUI_BT_SEARCH,     /* поиск: мигает */
    GUI_BT_CONNECTED,  /* соединение установлено: горит */
} gui_bt_t;

/* Запускает дисплей. vehicles - GUI_VEHICLES описаний, живут всё время работы. 0 - успех */
int gui_init(const gui_vehicle_t *vehicles);

void gui_set_battery(uint8_t level);            /* делений 0..5 */
void gui_set_bluetooth(gui_bt_t state);
void gui_set_joysticks(bool left, bool right);  /* какие физические джойконы подключены */
void gui_set_telemetry(uint8_t vehicle, const char *summary, const char *details);
void gui_set_message(const char *text);         /* NULL или "" - убрать */
void gui_select(uint8_t vehicle);
void gui_toggle_panels(void);

/* Касание; pressed = false - палец убран. Возвращает, куда попали */
screen_hit_t gui_touch(uint16_t x, uint16_t y, bool pressed);

/* Удерживаемая экранная кнопка или стик, иначе SCREEN_HIT_NONE */
screen_hit_t gui_pressed(void);

/* Вызывать раз в 10..50 мс: часы, мигание, перерисовка */
void gui_tick(uint32_t now_ms);

#if defined(__cplusplus)
}
#endif
