#include "ds18s20.h"

// Команды датчика (из даташита).
#define CMD_SKIP_ROM          0xCC   // обращаемся ко всем датчикам на шине
#define CMD_CONVERT_T         0x44   // начать измерение температуры
#define CMD_READ_SCRATCHPAD   0xBE   // прочитать память датчика

// Сколько байт в памяти датчика, последний из них это CRC.
#define SCRATCHPAD_SIZE       9

// Номера нужных байт в памяти датчика.
#define SP_TEMP_LSB           0      // температура, младший байт
#define SP_TEMP_MSB           1      // температура, старший байт
#define SP_COUNT_REMAIN       6      // COUNT_REMAIN
#define SP_COUNT_PER_C        7      // COUNT_PER_C, у датчика равен 16

// Время измерения по даташиту не больше 750 мс.
#define CONVERSION_TIME_MS    750

// Нога, к которой подключён датчик. Задаётся в ds18s20_init().
static GPIO_TypeDef *ds_port = 0;
static uint16_t ds_pin = 0;

// Задержка в микросекундах. Считаем такты ядра с помощью счётчика DWT,
// потому что HAL_Delay() умеет только в миллисекундах.
static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);

    while ((DWT->CYCCNT - start) < ticks)
    {
        // просто ждём
    }
}

// Прижать шину к земле.
static void bus_low(void)
{
    HAL_GPIO_WritePin(ds_port, ds_pin, GPIO_PIN_RESET);
}

// Отпустить шину. Дальше её поднимет резистор.
static void bus_release(void)
{
    HAL_GPIO_WritePin(ds_port, ds_pin, GPIO_PIN_SET);
}

// Прочитать текущий уровень на шине (1 или 0).
static int bus_read(void)
{
    return HAL_GPIO_ReadPin(ds_port, ds_pin) == GPIO_PIN_SET ? 1 : 0;
}

// Сброс шины. Возвращает 1, если датчик ответил импульсом присутствия,
// и 0, если никого нет.
static int onewire_reset(void)
{
    int present;

    __disable_irq();
    bus_low();
    delay_us(480);          // импульс сброса не меньше 480 мкс
    bus_release();
    delay_us(70);           // датчик отвечает через 15-60 мкс
    present = (bus_read() == 0);
    __enable_irq();

    delay_us(410);          // дожидаемся конца временного слота
    return present;
}

// Отправить один бит.
static void onewire_write_bit(int bit)
{
    __disable_irq();
    if (bit)
    {
        // единица: короткий импульс вниз и сразу отпускаем
        bus_low();
        delay_us(6);
        bus_release();
        delay_us(64);
    }
    else
    {
        // ноль: держим шину внизу почти весь слот
        bus_low();
        delay_us(60);
        bus_release();
        delay_us(10);
    }
    __enable_irq();
}

// Принять один бит.
static int onewire_read_bit(void)
{
    int bit;

    __disable_irq();
    bus_low();
    delay_us(6);
    bus_release();
    delay_us(9);            // датчик успевает выставить свой бит
    bit = bus_read();
    __enable_irq();

    delay_us(55);           // остаток слота
    return bit;
}

// Отправить байт. Сначала идёт младший бит.
static void onewire_write_byte(uint8_t value)
{
    for (int i = 0; i < 8; i++)
    {
        onewire_write_bit(value & 0x01);
        value >>= 1;
    }
}

// Принять байт. Сначала приходит младший бит.
static uint8_t onewire_read_byte(void)
{
    uint8_t value = 0;

    for (int i = 0; i < 8; i++)
    {
        if (onewire_read_bit())
        {
            value |= (uint8_t)(1U << i);
        }
    }
    return value;
}

// Запускает измерение, ждёт его конца и читает всю память датчика
// (9 байт) в массив scratchpad. Проверяет CRC.
static int read_scratchpad(uint8_t *scratchpad)
{
    // 1. Запускаем измерение
    if (!onewire_reset())
    {
        return DS18S20_ERR_NO_DEVICE;
    }
    onewire_write_byte(CMD_SKIP_ROM);
    onewire_write_byte(CMD_CONVERT_T);

    // 2. Ждём, пока датчик измерит
    HAL_Delay(CONVERSION_TIME_MS);

    // 3. Просим выдать память датчика
    if (!onewire_reset())
    {
        return DS18S20_ERR_NO_DEVICE;
    }
    onewire_write_byte(CMD_SKIP_ROM);
    onewire_write_byte(CMD_READ_SCRATCHPAD);

    for (int i = 0; i < SCRATCHPAD_SIZE; i++)
    {
        scratchpad[i] = onewire_read_byte();
    }

    // 4. Проверяем CRC. По всем 9 байтам (вместе с самой CRC) должен
    // получиться ноль.
    if (crc8_calc(scratchpad, SCRATCHPAD_SIZE) != 0)
    {
        return DS18S20_ERR_CRC;
    }

    return DS18S20_OK;
}

// Собирает знаковое 16-битное значение из байт 0 и 1 памяти.
static int16_t get_raw(const uint8_t *scratchpad)
{
    uint16_t value = (uint16_t)(((uint16_t)scratchpad[SP_TEMP_MSB] << 8)
                                | scratchpad[SP_TEMP_LSB]);
    return (int16_t)value;
}

void ds18s20_init(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    ds_port = port;
    ds_pin = pin;

    // Включаем счётчик тактов DWT. На Cortex-M7 его ещё надо разблокировать.
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // Открытый сток без внутренней подтяжки, подтяжка стоит снаружи.
    bus_release();
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

float ds18s20_calc_temperature(int16_t raw, uint8_t count_remain, uint8_t count_per_c)
{
    float temp_read;

    if (count_per_c == 0)
    {
        return (float)raw / 2.0f;
    }

    // Сдвиг вправо на 1 отбрасывает бит 0.5 (для минуса округляет вниз)
    temp_read = (float)(raw >> 1);

    return temp_read - 0.25f
           + (float)(count_per_c - count_remain) / (float)count_per_c;
}

int ds18s20_read_raw(int16_t *raw)
{
    uint8_t scratchpad[SCRATCHPAD_SIZE];
    int status;

    if (raw == 0)
    {
        return DS18S20_ERR_PARAM;
    }

    status = read_scratchpad(scratchpad);
    if (status != DS18S20_OK)
    {
        return status;
    }

    *raw = get_raw(scratchpad);
    return DS18S20_OK;
}

int ds18s20_read_temperature(float *celsius)
{
    uint8_t scratchpad[SCRATCHPAD_SIZE];
    int status;

    if (celsius == 0)
    {
        return DS18S20_ERR_PARAM;
    }

    status = read_scratchpad(scratchpad);
    if (status != DS18S20_OK)
    {
        return status;
    }

    *celsius = ds18s20_calc_temperature(get_raw(scratchpad),
                                        scratchpad[SP_COUNT_REMAIN],
                                        scratchpad[SP_COUNT_PER_C]);
    return DS18S20_OK;
}
