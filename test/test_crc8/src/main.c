/**
 * @file    main.c
 * @brief   Проверка модуля crc8 (CRC-8 Dallas/Maxim)
 *
 * @details Здесь я проверяю две функции из crc8.c:
 *          - crc8_update() - считает CRC по одному байту;
 *          - crc8_calc()   - считает CRC по целому массиву.
 *
 *          Ожидаемые значения я не придумывал, они взяты из
 *          проверенных источников:
 *          - стандартная проверочная строка "123456789" для
 *            CRC-8/MAXIM-DOW даёт 0xA1;
 *          - пример из документации Maxim (Application Note 27):
 *            ROM-код 02 1C B8 01 00 00 00 имеет CRC 0xA2.
 *
 *          Результат показывает светодиод (пин задан в main.h):
 *          - быстрое мигание (100 мс)  - все проверки прошли;
 *          - N коротких вспышек и пауза 2 секунды, дальше по кругу
 *            - не прошла проверка номер N (считай вспышки).
 *            Номера проверок идут в том порядке, как они вызываются
 *            в check(), то есть сверху вниз по этому файлу.
 */

#include "main.h"

/**
 * @brief   Настраивает пин светодиода как обычный выход
 *
 * @details Делаю так же, как в test_display: включаю тактирование
 *          порта, гашу светодиод и перевожу пин в режим push-pull.
 */
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

/**
 * @brief   Одна вспышка светодиода
 *
 * @param   on_ms   Сколько миллисекунд светодиод горит
 * @param   off_ms  Сколько миллисекунд потом не горит
 */
static void flash(uint32_t on_ms, uint32_t off_ms)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    HAL_Delay(on_ms);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    HAL_Delay(off_ms);
}

/** @brief Сколько проверок всего выполнено */
volatile uint32_t tests_total = 0;

/** @brief Сколько проверок не прошло */
volatile uint32_t tests_failed = 0;

/** @brief Номер первой упавшей проверки (0 - ничего не упало) */
volatile uint32_t first_failed_test = 0;

/**
 * @brief   Сравнивает полученное значение с ожидаемым
 *
 * @details Функция сама увеличивает счётчики и запоминает номер
 *          первой ошибки. Так в каждой проверке не надо писать
 *          один и тот же if.
 *
 * @param   name      Название проверки (чтобы понять, что упало)
 * @param   got       Что реально вернул наш код
 * @param   expected  Что должно было получиться
 */
static void check(const char *name, uint8_t got, uint8_t expected)
{
    
    (void)name;

    tests_total++;

    if (got != expected)
    {
        // Запоминаем номер только самой первой ошибки
        if (tests_failed == 0)
        {
            first_failed_test = tests_total;
        }

        tests_failed++;
    }
}

/**
 * @brief   Проверка пустых и некорректных входных данных
 *
 * @details Если данных нет, то CRC должна остаться равной начальному
 *          значению (0x00). Нулевой указатель тоже не должен
 *          ронять программу.
 */
static void test_empty_input(void)
{
    uint8_t dummy[1] = {0x55};

    // Длина 0: ни один байт не обрабатывается
    check("пустой массив (length = 0)", crc8_calc(dummy, 0), TEST_CRC8_EXPECT_EMPTY);

    // Нулевой указатель: функция должна просто вернуть начальное значение
    check("нулевой указатель", crc8_calc(0, 5), TEST_CRC8_EXPECT_EMPTY);
}

/**
 * @brief   Проверка CRC для одного байта
 *
 * @details Значения проверил отдельно: 0x00 даёт 0x00 (с нулевой
 *          начальной CRC ноль остаётся нулём), а остальные байты
 *          проверяют разные ветки в цикле по битам.
 */
static void test_single_byte(void)
{
    uint8_t b0 = 0x00;
    uint8_t b1 = 0x01;
    uint8_t b2 = 0x80;
    uint8_t b3 = 0xFF;

    check("один байт 0x00", crc8_calc(&b0, 1), TEST_CRC8_EXPECT_BYTE_00);
    check("один байт 0x01", crc8_calc(&b1, 1), TEST_CRC8_EXPECT_BYTE_01);
    check("один байт 0x80", crc8_calc(&b2, 1), TEST_CRC8_EXPECT_BYTE_80);
    check("один байт 0xFF", crc8_calc(&b3, 1), TEST_CRC8_EXPECT_BYTE_FF);
}

/**
 * @brief   Проверка по стандартной тестовой строке "123456789"
 *
 * @details Это общепринятый check-вектор для CRC. Для Dallas/Maxim
 *          правильный ответ 0xA1.
 */
static void test_check_string(void)
{
    const uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    check("строка \"123456789\"", crc8_calc(data, 9), TEST_CRC8_EXPECT_CHECK_STR);
}

/**
 * @brief   Проверка на примере из документации Maxim (1-Wire ROM)
 *
 * @details У 1-Wire устройства ROM-код состоит из 8 байт: 7 байт
 *          данных и последний байт - CRC от этих семи. Здесь два
 *          полезных свойства:
 *          1. CRC от первых 7 байт должна быть равна 0xA2;
 *          2. если посчитать CRC от всех 8 байт (вместе с самой CRC),
 *             должен получиться 0x00. Именно так приёмник проверяет,
 *             что данные дошли без ошибок.
 */
static void test_onewire_rom(void)
{
    const uint8_t rom[8] = {0x02, 0x1C, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xA2};

    check("ROM-код, 7 байт данных", crc8_calc(rom, 7), TEST_CRC8_EXPECT_ROM);
    check("ROM-код, все 8 байт (должен быть 0)", crc8_calc(rom, 8), TEST_CRC8_EXPECT_WITH_CRC);
}

/**
 * @brief   Проверка, что CRC замечает ошибку в данных
 *
 * @details Беру правильный пакет вместе с CRC и портю в нём один бит.
 *          После этого итог уже не должен быть нулевым. CRC-8
 *          гарантированно ловит все однобитные ошибки.
 */
static void test_error_detection(void)
{
    uint8_t rom[8] = {0x02, 0x1C, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xA2};
    uint8_t result;

    // Переворачиваем один бит в третьем байте
    rom[2] = rom[2] ^ 0x04;

    result = crc8_calc(rom, 8);

    // Тут не сравниваем с конкретным числом, нам важно только "не 0".
    // Поэтому ожидаемое значение передаём как результат условия.
    check("ошибка в одном бите обнаружена", (uint8_t)(result != 0), 1);
}

/**
 * @brief   Проверка, что crc8_update() и crc8_calc() согласованы
 *
 * @details Если вручную прогнать байты через crc8_update(), начиная
 *          с CRC8_INIT_VALUE, то должен получиться тот же результат,
 *          что и у crc8_calc(). Это важно, если данные приходят
 *          по кусочкам (например по UART) и целого массива нет.
 */
static void test_update_vs_calc(void)
{
    const uint8_t data[] = {0x10, 0x20, 0x30, 0x40, 0x50};
    uint8_t crc = CRC8_INIT_VALUE;

    for (uint32_t i = 0; i < 5; i++)
    {
        crc = crc8_update(crc, data[i]);
    }

    check("update по байтам == calc по массиву", crc, crc8_calc(data, 5));
}

/**
 * @brief   Точка входа теста
 *
 * @details Сначала настраиваю плату как в других тестах, потом
 *          запускаю все проверки и показываю итог светодиодом.
 *          Из main на микроконтроллере выходить нельзя, поэтому
 *          везде while (1).
 *
 * @return  Не возвращается, тип int нужен по стандарту C
 */
int main(void)
{
    MPU_Config();
    HAL_Init();
    SystemClock_Config();
    led_init();

    test_empty_input();
    test_single_byte();
    test_check_string();
    test_onewire_rom();
    test_error_detection();
    test_update_vs_calc();

    // Всё прошло: быстро мигаем
    if (tests_failed == 0)
    {
        while (1)
        {
            flash(100, 100);
        }
    }

    // Что-то упало: мигаем столько раз, какой номер у первой ошибки
    while (1)
    {
        for (uint32_t i = 0; i < first_failed_test; i++)
        {
            flash(200, 300);
        }

        // Пауза, чтобы было понятно, где кончилась серия вспышек
        HAL_Delay(2000);
    }
}
