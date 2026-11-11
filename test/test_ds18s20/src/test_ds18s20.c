#include "main.h"

/**
 * @file main.c
 * @brief Тест модуля ds18s20.
 *
 * Тест состоит из двух частей.
 *
 * Часть 1. Проверка формулы уточнения температуры без железа. Считаем
 * несколько значений вручную и сравниваем с ds18s20_calc_temperature().
 *
 * Часть 2. Проверка с настоящим датчиком. Раз в секунду измеряем
 * температуру и печатаем её в UART7 (1000000 бод). Если температура
 * получена и лежит в пределах рабочего диапазона датчика
 * (от -55 до +125 градусов), то светодиод на PE3 переключается.
 * Если пришла ошибка, светодиод гаснет, а в терминал выводится её код.
 */

/** @brief Нога светодиода на плате. */
#define LED_PORT        GPIOE
#define LED_PIN         GPIO_PIN_3

// Куда подключена линия DATA датчика DS18S20.
// Между DATA и 3.3 В нужен резистор 4.7 кОм.
#define DS_PORT         GPIOB
#define DS_PIN          GPIO_PIN_0

/** @brief Границы рабочего диапазона датчика в градусах. */
#define TEMP_MIN        (-55.0f)
#define TEMP_MAX        (125.0f)

void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();   // порт, где DS_PORT

    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

// Проверка формулы температуры. Сравниваем в сотых долях градуса,
// чтобы не сравнивать float напрямую.
static int check_calc(int16_t raw, uint8_t remain, uint8_t per_c, int expected_x100)
{
    float t = ds18s20_calc_temperature(raw, remain, per_c);
    int t_x100 = (int)(t * 100.0f);

    if (t_x100 != expected_x100)
    {
        printf("calc(%d, %d, %d): got %d, expected %d\r\n",
               raw, remain, per_c, t_x100, expected_x100);
        return 0;
    }
    return 1;
}

static int test_calc(void)
{
    int ok = 1;

    // +25.0: raw = 50, COUNT_REMAIN = 12, COUNT_PER_C = 16 -> 25 - 0.25 + 0.25
    ok &= check_calc(50, 12, 16, 2500);
    // -25.0: raw = -50, те же счётчики -> -25 - 0.25 + 0.25
    ok &= check_calc(-50, 12, 16, -2500);
    // +25.5: raw = 51, TEMP_READ = 25, COUNT_REMAIN = 8 -> 25 - 0.25 + 0.5
    ok &= check_calc(51, 8, 16, 2525);
    // Если COUNT_PER_C равен 0, берём просто raw / 2
    ok &= check_calc(51, 0, 0, 2550);

    if (ok)
    {
        printf("Calc tests: PASS\r\n");
    }
    else
    {
        printf("Calc tests: FAIL\r\n");
    }
    return ok;
}

// Печать температуры без float в printf
static void print_temperature(float t)
{
    int t_x100 = (int)(t * 100.0f);
    char sign = '+';

    if (t_x100 < 0)
    {
        sign = '-';
        t_x100 = -t_x100;
    }

    printf("T = %c%d.%02d C\r\n", sign, t_x100 / 100, t_x100 % 100);
}

int main(void)
{
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    stdio_uart_init();

    printf("\r\nDS18S20 test\r\n");

    // Часть 1: формула
    test_calc();

    // Часть 2: датчик
    ds18s20_init(DS_PORT, DS_PIN);

    while (1)
    {
        float t = 0.0f;
        int status = ds18s20_read_temperature(&t);

        if (status == DS18S20_OK && t >= TEMP_MIN && t <= TEMP_MAX)
        {
            print_temperature(t);
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        }
        else
        {
            printf("Sensor error, code = %d\r\n", status);
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        }

        HAL_Delay(1000);
    }
}
