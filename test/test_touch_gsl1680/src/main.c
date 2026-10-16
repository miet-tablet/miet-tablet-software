#include "main.h"

/* Example wiring for STM32H7B0VBT6; check the actual board pinout. */
#define LED_PORT     GPIOE
#define LED_PIN      GPIO_PIN_3
#define TOUCH_RST    GPIO_PIN_6  /* PC6 -> JP1 pin 36 */

/* Requires I2C1 clock source = PCLK1 = 140 MHz in the current project. */
#define TOUCH_I2C_TIMING 0xD0622C36U /* approximately 100 kHz; scope-check */

/* For a mounted screen swap/invert axes here if necessary. */
#define TOUCH_SWAP_XY  0
#define TOUCH_INVERT_X 0
#define TOUCH_INVERT_Y 0

volatile int touch_test_stage = 0;  /* 0 OK; 1..5 indicate a failure */
volatile int touch_driver_error = 0;
volatile uint16_t touch_x = 0;
volatile uint16_t touch_y = 0;
volatile uint8_t touch_pressed = 0;
volatile uint8_t touch_contacts = 0;

static I2C_HandleTypeDef hi2c1;
static gsl1680_t touch_device;

static void led_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOE_CLK_ENABLE();
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    gpio.Pin = LED_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &gpio);
}

static void flash(uint32_t on_ms, uint32_t off_ms)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    HAL_Delay(on_ms);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    HAL_Delay(off_ms);
}

static void blink_failure(int count)
{
    for (int i = 0; i < count; ++i) flash(130, 170);
    HAL_Delay(1000);
}

static int touch_i2c_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* PB8 = SCL, PB9 = SDA, AF4, open-drain; external 3V3 pullups needed. */
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PC6 is active-low hardware reset of touch controller. */
    HAL_GPIO_WritePin(GPIOC, TOUCH_RST, GPIO_PIN_SET);
    gpio.Pin = TOUCH_RST;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &gpio);

    if (HAL_RCC_GetPCLK1Freq() != 140000000U) return -1;

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = TOUCH_I2C_TIMING;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    return HAL_I2C_Init(&hi2c1) == HAL_OK ? 0 : -1;
}

static void convert_coordinates(uint16_t *x, uint16_t *y)
{
    (void)x;
    (void)y;
#if TOUCH_SWAP_XY
    uint16_t t = *x;
    *x = *y;
    *y = t;
#endif
#if TOUCH_INVERT_X
    if (*x < RA8875_WIDTH) *x = RA8875_WIDTH - 1U - *x;
#endif
#if TOUCH_INVERT_Y
    if (*y < RA8875_HEIGHT) *y = RA8875_HEIGHT - 1U - *y;
#endif
}

int main(void)
{
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    led_init();

    touch_test_stage = 1;
    if (ra8875_init() != 0) goto failure;

    ra8875_fill_screen(RA8875_BLACK);
    ra8875_draw_text(12, 12, "TOUCH: draw with one finger", RA8875_WHITE, RA8875_BLACK, 1);

    touch_test_stage = 2;
    if (touch_i2c_init() != 0) goto failure;

    touch_device.i2c = &hi2c1;
    touch_device.reset_port = GPIOC;
    touch_device.reset_pin = TOUCH_RST;
    touch_device.ready = false;

    touch_driver_error = gsl1680_init(&touch_device, gsl1680_firmware,
                                      gsl1680_firmware_count);
    if (touch_driver_error != 0)
    {
        touch_test_stage = -touch_driver_error + 2;
        goto failure;
    }

    touch_test_stage = 0;
    uint16_t prev_x = 0;
    uint16_t prev_y = 0;
    bool was_pressed = false;

    while (1)
    {
        gsl1680_touch_t sample = {0};
        touch_driver_error = gsl1680_read(&touch_device, &sample);
        if (touch_driver_error != 0)
        {
            touch_test_stage = 10;
            goto failure;
        }

        touch_contacts = sample.contacts;
        touch_pressed = sample.pressed ? 1U : 0U;
        touch_x = sample.x;
        touch_y = sample.y;

        uint16_t x = sample.x;
        uint16_t y = sample.y;
        convert_coordinates(&x, &y);

        if (sample.pressed && x < RA8875_WIDTH && y < RA8875_HEIGHT &&
            (!was_pressed || x != prev_x || y != prev_y))
        {
            ra8875_fill_circle(x, y, 3, RA8875_GREEN);
        }
        prev_x = x;
        prev_y = y;
        was_pressed = sample.pressed;

        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(25);
    }

failure:
    while (1) blink_failure(touch_test_stage);
}
