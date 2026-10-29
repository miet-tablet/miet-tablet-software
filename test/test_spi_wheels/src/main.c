#include "main.h"

/*
 * Проверка SPI1 (модуль spi_wheels) без внешнего устройства, по петле.
 *
 * Подключение: перемычка PD7 (MOSI) <-> PB4 (MISO).
 * SPI работает в полном дуплексе: что уходит по MOSI, то же самое
 * одновременно должно приходить по MISO.
 *
 * Тесты:
 *   1 - spi_wheels_init(): SPI1 и его пины настроились
 *   2 - короткий шаблон: 0x00/0xFF ловят линию, залипшую в 0 или 1,
 *       0x55/0xAA/0xA5/0x5A - замыкание соседних битов,
 *       0x01/0x80 - перепутанный порядок бит
 *   3 - 256 байт подряд (0..255): посылка длиннее FIFO SPI (16 байт),
 *       сдвиг или потеря байта сразу видны
 *
 * Индикация светодиодом (PE3):
 *   одна короткая вспышка за цикл  - тесты 2 и 3 прошли
 *   N вспышек подряд               - упал тест номер N
 * Подробности печатаются в UART7 (PE8 = TX, PE7 = RX, 1 Мбод, 8N1).
 *
 * Тесты повторяются каждые TEST_PERIOD_MS, поэтому перемычку можно
 * снимать на ходу: индикация должна смениться на ошибку.
 *
 * Осциллограф: синхронизация по спаду CS (PD6), щупы на SCK (PB3)
 * и MOSI (PD7). SCK = 280 МГц / 64 ~ 4.4 МГц (расчётно), режим 0
 * (CPOL = 0, CPHA = 0), старший бит первый. За цикл CS опускается
 * два раза: сначала 8 байт шаблона, затем 256 байт счётчика.
 */

#define TEST_PERIOD_MS  500
#define LONG_TEST_LEN   256

/* Номера тестов = число вспышек при ошибке */
#define TEST_INIT       1
#define TEST_PATTERN    2
#define TEST_LONG       3

static const uint8_t pattern[] = {0x00, 0xFF, 0x55, 0xAA, 0xA5, 0x5A, 0x01, 0x80};

static uint8_t long_tx[LONG_TEST_LEN];

/* Общий приёмный буфер для обоих тестов */
static uint8_t rx[LONG_TEST_LEN];

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

static void blink_failure(int test)
{
    for (int i = 0; i < test; i++)
    {
        flash(150, 150);
    }
}

/*
 * Отправляет len байт из tx, принимает их в rx и сравнивает.
 * Возвращает 0, если всё совпало, иначе -1.
 */
static int loopback_check(const char *name, const uint8_t *tx, int len)
{
    /* Заполняем rx инверсией tx, чтобы старые данные не совпали случайно */
    for (int i = 0; i < len; i++)
    {
        rx[i] = (uint8_t)~tx[i];
    }

    if (spi_wheels_transfer(tx, rx, len) != 0)
    {
        printf("%s: FAIL, HAL error\n", name);
        return -1;
    }

    for (int i = 0; i < len; i++)
    {
        if (rx[i] != tx[i])
        {
            printf("%s: FAIL, byte %d: sent 0x%02X, received 0x%02X\n",
                   name, i, tx[i], rx[i]);
            return -1;
        }
    }

    printf("%s: OK, %d bytes\n", name, len);
    return 0;
}

int main(void)
{
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    led_init();
    stdio_uart_init();

    printf("\nSPI1 loopback test, connect PD7 (MOSI) to PB4 (MISO)\n");

    if (spi_wheels_init() != 0)
    {
        printf("spi_wheels_init: FAIL\n");
        while (1)
        {
            blink_failure(TEST_INIT);
            HAL_Delay(1000);
        }
    }

    for (int i = 0; i < LONG_TEST_LEN; i++)
    {
        long_tx[i] = (uint8_t)i;
    }

    while (1)
    {
        /* Оба теста запускаются всегда, чтобы картинка на осциллографе не менялась */
        int pattern_status = loopback_check("pattern", pattern, (int)sizeof(pattern));
        int long_status = loopback_check("long", long_tx, LONG_TEST_LEN);

        if (pattern_status != 0)
        {
            blink_failure(TEST_PATTERN);
        }
        else if (long_status != 0)
        {
            blink_failure(TEST_LONG);
        }
        else
        {
            flash(50, 0);
        }

        HAL_Delay(TEST_PERIOD_MS);
    }
}
