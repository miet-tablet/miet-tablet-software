#include <stdio.h>
#include "stm32h7xx_hal.h"
#include "ina3221_barsotion.h"
#include "ina3221_regconst.h"
#include "ina3221_types.h"

I2C_HandleTypeDef hi2c2;
#define INA3221_I2C_ADDR (0x40 << 1)

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C2_Init(void);
void Error_Handler(void);

// Обертки для драйвера (передача данных по I2C)
int my_ina_read(uint8_t reg, uint8_t *data, unsigned len) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, INA3221_I2C_ADDR, reg, 1, data, len, 100);
    return (status == HAL_OK) ? 0 : -1;
}

int my_ina_write(uint8_t reg, const uint8_t *data, unsigned len) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c2, INA3221_I2C_ADDR, reg, 1, (uint8_t*)data, len, 100);
    return (status == HAL_OK) ? 0 : -1;
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C2_Init();

    struct ina3221_cfg cfg = {
        .read_register = my_ina_read,
        .write_register = my_ina_write
    };
    struct ina3221_desc ina;

    printf("INA3221 Test Starting...\r\n");

    if (ina3221_init(&ina, &cfg) != 0) {
        printf("INA3221 Init Failed!\r\n");
        Error_Handler();
    }

    // Включаем 1 канал (Continuous mode)
    struct ina3221_config config = {0};
    config.mode = MODE_SHUNT_AND_BUS_CONTINIOUS;
    config.ch1_en = 1;
    config.averaging_mode = AVG_4;
    if (ina3221_set_config(&ina, config) != 0) {
        printf("Config Failed!\r\n");
        Error_Handler();
    }

    HAL_Delay(10); // Даем время на первое измерение

    while (1) {
        int16_t bus_raw = 0;
        int16_t shunt_raw = 0;

        if (ina3221_get_ch1_bus_voltage(&ina, &bus_raw) == 0 &&
            ina3221_get_ch1_shunt_voltage(&ina, &shunt_raw) == 0) {
            
            // Bus Voltage LSB = 8mV
            float voltage = bus_raw * 0.008f;
            
            // Shunt Voltage LSB = 40uV. 
            // Уточните сопротивление шунта (R_shunt) на вашей плате! Пример 0.1 Ом
            float r_shunt = 0.1f; 
            float current = (shunt_raw * 0.00004f) / r_shunt;

            printf("CH1 -> Voltage: %.2f V | Current: %.3f A\r\n", voltage, current);
        } else {
            printf("Read Error\r\n");
        }

        HAL_Delay(1000);
    }
}

// --- Системные функции (скопированы из test_i2c_wheels) ---

void MX_I2C2_Init(void) {
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x10C0ECFF; 
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        Error_Handler();
    }
}

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOE_CLK_ENABLE();
    // Настройка пина светодиода (если нужно)
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void Error_Handler(void) {
    __disable_irq();
    while (1);
}

void SysTick_Handler(void) {
    HAL_IncTick();
}

void SystemClock_Config(void) {
    // Упрощенная настройка (скопируйте полную версию из test_i2c_wheels, если нужно)
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 112;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) Error_Handler();
}

// Если printf не работает, добавьте сюда:
// int __io_putchar(int ch) { HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF); return ch; }
