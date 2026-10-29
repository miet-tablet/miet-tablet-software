#include "main.h"

/*
 * Демо GUI (описание в README.md). Данные выдуманные:
 *   заряд батареи - минус деление каждые 3 с;
 *   Bluetooth - по 6 с: нет, поиск, подключён;
 *   джойконы - по 10 с: нет, левый, оба (экранное управление убирается), правый;
 *   телеметрия - меняется раз в 0,5 с.
 * Светодиод: мигает раз в секунду - работает, часто - дисплей не найден.
 */

#define LOOP_MS           20

/* Тач как в test_touch_gsl1680: I2C1 на PB8/PB9, сброс PC6 */
#define TOUCH_RST         GPIO_PIN_6
#define TOUCH_I2C_TIMING  0xD0622C36U  /* ~100 кГц при PCLK1 = 140 МГц */

/* Если касания не совпадают с картинкой, поменять оси здесь */
#define TOUCH_SWAP_XY     0
#define TOUCH_INVERT_X    0
#define TOUCH_INVERT_Y    0

/* Заглушки назначений вида <изделие>_btn_L_up, как в ТЗ */
#define BUTTONS(v, side) \
    {v "_btn_" side "_up", v "_btn_" side "_left", v "_btn_" side "_right", v "_btn_" side "_down"}

#define VEHICLE(title, v)                         \
    {                                             \
        .name = title,                            \
        .buttons = {BUTTONS(v, "L"), BUTTONS(v, "R")}, \
        .sticks = {v "_stick_L", v "_stick_R"},   \
    }

static const gui_vehicle_t vehicles[GUI_VEHICLES] = {
    VEHICLE("Samosval", "samosval"),
    VEHICLE("Ekskavator", "ekskavator"),
    VEHICLE("Mashinka", "mashinka"),
};

static I2C_HandleTypeDef hi2c1;
static gsl1680_t touch_device;
static bool touch_ok;

static void led_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    LED_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    gpio.Pin = LED_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &gpio);
}

static int touch_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* PB8 = SCL, PB9 = SDA, открытый сток, нужны внешние подтяжки к 3,3 В */
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PC6 - сброс контроллера, активный низкий */
    HAL_GPIO_WritePin(GPIOC, TOUCH_RST, GPIO_PIN_SET);
    gpio.Pin = TOUCH_RST;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOC, &gpio);

    if (HAL_RCC_GetPCLK1Freq() != 140000000U)
    {
        return -1;
    }

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = TOUCH_I2C_TIMING;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        return -1;
    }

    touch_device.i2c = &hi2c1;
    touch_device.reset_port = GPIOC;
    touch_device.reset_pin = TOUCH_RST;
    touch_device.ready = false;

    return gsl1680_init(&touch_device, gsl1680_firmware, gsl1680_firmware_count);
}

static void touch_poll(void)
{
    gsl1680_touch_t sample = {0};

    if (!touch_ok)
    {
        return;
    }
    if (gsl1680_read(&touch_device, &sample) != 0)
    {
        touch_ok = false;
        gui_set_message("touch read error");
        return;
    }

    uint16_t x = sample.x;
    uint16_t y = sample.y;

#if TOUCH_SWAP_XY
    uint16_t t = x;
    x = y;
    y = t;
#endif
#if TOUCH_INVERT_X
    x = x < GUI_WIDTH ? GUI_WIDTH - 1U - x : x;
#endif
#if TOUCH_INVERT_Y
    y = y < GUI_HEIGHT ? GUI_HEIGHT - 1U - y : y;
#endif

    gui_touch(x, y, sample.pressed);
}

static void demo_status(uint32_t now_ms)
{
    static const bool left[4] = {false, true, true, false};
    static const bool right[4] = {false, false, true, true};
    uint32_t s = now_ms / 1000;
    uint32_t j = (s / 10) % 4;

    gui_set_battery(5 - (s / 3) % 6);
    gui_set_bluetooth((gui_bt_t)((s / 6) % 3));
    gui_set_joysticks(left[j], right[j]);
}

static void demo_telemetry(uint32_t now_ms)
{
    char summary[GUI_SUMMARY_LEN];
    char details[GUI_DETAILS_LEN];
    unsigned long t = now_ms / 500;

    for (uint8_t i = 0; i < GUI_VEHICLES; i++)
    {
        /* У каждого изделия свои числа, лишь бы менялись */
        unsigned long mv = 7000 + (t * 7 + i * 300) % 1400;
        unsigned long speed = (t * 3 + i * 17) % 100;
        unsigned long rpm = (t * 11 + i * 40) % 300;
        unsigned long temp = 30 + (t / 4 + i * 5) % 20;

        snprintf(summary, sizeof(summary), "U %lu.%02lu V  v %lu cm/s",
                 mv / 1000, mv % 1000 / 10, speed);
        snprintf(details, sizeof(details),
                 "Battery: %lu.%02lu V\n"
                 "Speed:   %lu cm/s\n"
                 "Motor L: %lu rpm\n"
                 "Motor R: %lu rpm\n"
                 "Temp:    %lu C\n"
                 "Uptime:  %lu s\n"
                 "Link:    demo data",
                 mv / 1000, mv % 1000 / 10, speed, rpm, rpm + 5, temp,
                 (unsigned long)(now_ms / 1000));

        gui_set_telemetry(i, summary, details);
    }
}

int main(void)
{
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    led_init();

    if (gui_init(vehicles) != 0)
    {
        while (1)
        {
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
            HAL_Delay(100);
        }
    }

    /* Сначала картинка, потом долгая загрузка прошивки тача */
    gui_tick(HAL_GetTick());
    touch_ok = touch_init() == 0;
    if (!touch_ok)
    {
        gui_set_message("touch not found");
    }

    uint32_t led_ms = 0;

    while (1)
    {
        uint32_t now = HAL_GetTick();

        demo_status(now);
        demo_telemetry(now);
        touch_poll();
        gui_tick(now);

        if (now - led_ms >= 500)
        {
            led_ms = now;
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        }
        HAL_Delay(LOOP_MS);
    }
}
