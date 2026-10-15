#include "main.h"

/*
 * Draws a test pattern on the RA8875 display and reports with the LED
 * (pin in main.h):
 *   fast blink (100 ms)      - display initialized, pattern drawn
 *   double flash every 1 s   - no RA8875, but MISO-MOSI loopback works
 *                              (PB14 shorted to PB15: SPI2 itself is fine)
 *   slow blink (1 s)         - neither: check wiring and SPI pins
 *
 * Pattern, top to bottom:
 *   8 colour bars with a circle and a triangle on them (proportions),
 *   a grey ramp written pixel by pixel (memory writes, RGB565 byte order),
 *   a line of text in the built-in font,
 *   and a red + white frame on the outermost pixels (visible area).
 */

#define BARS_HEIGHT  360
#define RAMP_TOP     360
#define RAMP_HEIGHT  60
#define TEXT_TOP     (RAMP_TOP + RAMP_HEIGHT)

static uint16_t ramp[RA8875_WIDTH];

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

static void flash(uint32_t on_ms, uint32_t off_ms)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    HAL_Delay(on_ms);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    HAL_Delay(off_ms);
}

static void draw_test_pattern(void)
{
    static const uint16_t bar_colors[8] = {
        RA8875_WHITE, RA8875_YELLOW, RA8875_CYAN, RA8875_GREEN,
        RA8875_MAGENTA, RA8875_RED, RA8875_BLUE, RA8875_BLACK,
    };
    const uint16_t bar_width = RA8875_WIDTH / 8;

    for (uint16_t i = 0; i < 8; i++)
    {
        ra8875_fill_rect(i * bar_width, 0, bar_width, BARS_HEIGHT, bar_colors[i]);
    }

    /* A round circle and a symmetric triangle mean the aspect ratio is right */
    ra8875_draw_circle(RA8875_WIDTH / 2, BARS_HEIGHT / 2, 150, RA8875_BLACK);
    ra8875_fill_triangle(710, 300, 790, 300, 750, 240, RA8875_WHITE);

    for (uint16_t x = 0; x < RA8875_WIDTH; x++)
    {
        uint8_t level = (uint8_t)(x * 255 / (RA8875_WIDTH - 1));
        ramp[x] = RA8875_RGB565(level, level, level);
    }
    for (uint16_t y = RAMP_TOP; y < RAMP_TOP + RAMP_HEIGHT; y++)
    {
        ra8875_write_pixels(0, y, ramp, RA8875_WIDTH);
    }

    ra8875_fill_rect(0, TEXT_TOP, RA8875_WIDTH, RA8875_HEIGHT - TEXT_TOP, RA8875_BLACK);
    ra8875_draw_text(16, TEXT_TOP + 12, "MIET tablet  800x480  RA8875",
                     RA8875_WHITE, RA8875_BLACK, 1);

    /* Two nested frames: on every bar at least one of them stands out */
    ra8875_draw_rect(0, 0, RA8875_WIDTH, RA8875_HEIGHT, RA8875_RED);
    ra8875_draw_rect(1, 1, RA8875_WIDTH - 2, RA8875_HEIGHT - 2, RA8875_WHITE);
}

int main(void)
{
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    led_init();

    if (ra8875_init() == 0)
    {
        draw_test_pattern();
        while (1)
        {
            flash(100, 100);
        }
    }

    if (ra8875_port_loopback_test() == 0)
    {
        while (1)
        {
            flash(100, 150);
            flash(100, 650);
        }
    }

    while (1)
    {
        flash(1000, 1000);
    }
}
