#include "main.h"

#define LED_PORT GPIOE
#define LED_PIN  GPIO_PIN_3

#define ICM45686_REG_ACCEL_CONFIG0 0x1B
#define ICM45686_REG_GYRO_CONFIG0  0x1C
#define ICM45686_REG_WHO_AM_I      0x72
#define ICM45686_WHO_AM_I_VALUE    0xE9

volatile int failed_test = 0;       //Номер первого не пройденного теста 
volatile uint8_t who_am_i = 0;

static void led_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);

    gpio.Pin  = LED_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(LED_PORT, &gpio);
}

static void led_blink_code(int count)       //номер не пройденного теста  = короткие мигания
{
    for (int i = 0; i < count; i++)
    {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(150);

        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(150);
    }

    HAL_Delay(1500);
}

//Тест 1: инициализация SPI
static int test_init(void)
{
    return spi_icm45686_init() == 0;
}

// Тест 2: проверка связи с ICM-45686
static int test_who_am_i(void)
{
    uint8_t value = 0;

    if (icm45686_read_register(ICM45686_REG_WHO_AM_I, &value, 1) != 0)
    {
        return 0;
    }

    who_am_i = value;

    return value == ICM45686_WHO_AM_I_VALUE;
}

// Тест 3: запись значения в регистр и чтение его обратно 
static int test_write_read(void)
{
    uint8_t original = 0;
    uint8_t test_value;
    uint8_t read_value = 0;
    int result = 0;

    if (icm45686_read_register(ICM45686_REG_ACCEL_CONFIG0, &original, 1) != 0)
    {
        return 0;
    }

    /*
     * Меняем только ACCEL_ODR [3:0].
     * 0x07 = 400 Hz, 0x08 = 200 Hz.
     * Биты диапазона акселерометра [6:4] сохраняем.
     */
    test_value = (original & 0x70) | (((original & 0x0F) == 0x07) ? 0x08 : 0x07);

    if (icm45686_write_register(ICM45686_REG_ACCEL_CONFIG0, &test_value, 1) != 0)
    {
        return 0;
    }

    if (icm45686_read_register(ICM45686_REG_ACCEL_CONFIG0, &read_value, 1) == 0 &&
        read_value == test_value)
    {
        result = 1;
    }

    // Восстанавление исходного значения регистра
    if (icm45686_write_register(ICM45686_REG_ACCEL_CONFIG0, &original, 1) != 0)
    {
        return 0;
    }

    return result;
}

// Тест 4: проверка многобайтового чтения 
static int test_burst_read(void)
{
    uint8_t burst[2] = {0};
    uint8_t accel_config = 0;
    uint8_t gyro_config = 0;

    // Чтение двух регистров одним обменом
    if (icm45686_read_register(ICM45686_REG_ACCEL_CONFIG0, burst, 2) != 0)
    {
        return 0;
    }

    // Чтение их же, но отдельно
    if (icm45686_read_register(ICM45686_REG_ACCEL_CONFIG0, &accel_config, 1) != 0)
    {
        return 0;
    }
    if (icm45686_read_register(ICM45686_REG_GYRO_CONFIG0, &gyro_config, 1) != 0)
    {
        return 0;
    }

    return burst[0] == accel_config && burst[1] == gyro_config;
}

// Запуск след теста только если предыдущий прошел
static void run_test(int number, int (*test)(void))
{
    if (failed_test == 0 && !test())
    {
        failed_test = number;
    }
}

int main(void)
{
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    led_init();

    HAL_Delay(10);

    run_test(1, test_init);
    run_test(2, test_who_am_i);
    run_test(3, test_write_read);
    run_test(4, test_burst_read);

    while (1)
    {
        if (failed_test == 0)
        {
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);     //все тесты прошли
            HAL_Delay(500);
        }
        else
        {
            led_blink_code(failed_test);
        }
    }
}