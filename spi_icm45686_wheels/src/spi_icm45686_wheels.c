#include "spi_icm45686_wheels.h"

#define SPI_WHEELS_CS_PORT  GPIOD
#define SPI_WHEELS_CS_PIN   GPIO_PIN_6
#define SPI_WHEELS_TIMEOUT  100

static SPI_HandleTypeDef spi_host;

int spi_icm45686_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    //SCK (PB3), MISO (PB4)
    gpio.Pin = GPIO_PIN_3 | GPIO_PIN_4;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &gpio);

    //MOSI (PD7) 
    gpio.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOD, &gpio);

    //CS (PD6): сначала 1, потом выход 
    HAL_GPIO_WritePin(SPI_WHEELS_CS_PORT, SPI_WHEELS_CS_PIN, GPIO_PIN_SET);
    gpio.Pin = SPI_WHEELS_CS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(SPI_WHEELS_CS_PORT, &gpio);

    //SPI1
    spi_host.Instance = SPI1;
    spi_host.Init.Mode = SPI_MODE_MASTER;
    spi_host.Init.Direction = SPI_DIRECTION_2LINES;
    spi_host.Init.DataSize = SPI_DATASIZE_8BIT;
    spi_host.Init.CLKPolarity = SPI_POLARITY_LOW;
    spi_host.Init.CLKPhase = SPI_PHASE_1EDGE;
    spi_host.Init.NSS = SPI_NSS_SOFT;
    spi_host.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    spi_host.Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi_host.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;

    return HAL_SPI_Init(&spi_host) == HAL_OK ? 0 : -1;
}

int icm45686_read_register(uint8_t address, uint8_t *value, int len)
{
    uint8_t cmd = address | 0x80;
    HAL_StatusTypeDef status;

    HAL_GPIO_WritePin(SPI_WHEELS_CS_PORT, SPI_WHEELS_CS_PIN, GPIO_PIN_RESET);

    status = HAL_SPI_Transmit(&spi_host, &cmd, 1, SPI_WHEELS_TIMEOUT);
    if (status == HAL_OK)
    {
        status = HAL_SPI_Receive(&spi_host, value, len, SPI_WHEELS_TIMEOUT);
    }

    HAL_GPIO_WritePin(SPI_WHEELS_CS_PORT, SPI_WHEELS_CS_PIN, GPIO_PIN_SET);

    return status == HAL_OK ? 0 : -1;
}

int icm45686_write_register(uint8_t address, const uint8_t *value, int len)
{
    uint8_t cmd = address & 0x7F;
    HAL_StatusTypeDef status;

    HAL_GPIO_WritePin(SPI_WHEELS_CS_PORT, SPI_WHEELS_CS_PIN, GPIO_PIN_RESET);

    status = HAL_SPI_Transmit(&spi_host, &cmd, 1, SPI_WHEELS_TIMEOUT);
    if (status == HAL_OK)
    {
        status = HAL_SPI_Transmit(&spi_host, value, len, SPI_WHEELS_TIMEOUT);
    }

    HAL_GPIO_WritePin(SPI_WHEELS_CS_PORT, SPI_WHEELS_CS_PIN, GPIO_PIN_SET);

    return status == HAL_OK ? 0 : -1;
}