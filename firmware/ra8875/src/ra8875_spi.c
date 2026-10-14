#include "ra8875_spi.h"

#define RA8875_SPI_TIMEOUT    10

/*
 * The kernel clock is pll1_q (280 MHz after SystemClock_Config).
 * /256 = ~1.1 MHz: slow enough for the RA8875 before its PLL is configured.
 * /64 = ~4.4 MHz: about what the vendor's example runs at after init.
 */
#define RA8875_SPI_PRESCALER_SLOW  SPI_BAUDRATEPRESCALER_256
#define RA8875_SPI_PRESCALER_FAST  SPI_BAUDRATEPRESCALER_64

static SPI_HandleTypeDef spi_host;

static void cs_low(void)
{
    HAL_GPIO_WritePin(RA8875_CS_PORT, RA8875_CS_PIN, GPIO_PIN_RESET);
}

static void cs_high(void)
{
    HAL_GPIO_WritePin(RA8875_CS_PORT, RA8875_CS_PIN, GPIO_PIN_SET);
}

static void write_cycle(uint8_t type, uint8_t value)
{
    uint8_t tx[2] = {type, value};

    cs_low();
    HAL_SPI_Transmit(&spi_host, tx, 2, RA8875_SPI_TIMEOUT);
    cs_high();
}

static uint8_t read_cycle(uint8_t type)
{
    uint8_t tx[2] = {type, 0x00};
    uint8_t rx[2] = {0};

    cs_low();
    HAL_SPI_TransmitReceive(&spi_host, tx, rx, 2, RA8875_SPI_TIMEOUT);
    cs_high();

    return rx[1];
}

int ra8875_port_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RA8875_SPI_CLK_ENABLE();
    RA8875_SPI_GPIO_CLK_ENABLE();
    RA8875_CS_CLK_ENABLE();

    gpio.Pin = RA8875_SCK_PIN | RA8875_MOSI_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = RA8875_SPI_AF;
    HAL_GPIO_Init(RA8875_SPI_GPIO_PORT, &gpio);

    /* Pull-up: with no display attached MISO reads 0xFF instead of noise */
    gpio.Pin = RA8875_MISO_PIN;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(RA8875_SPI_GPIO_PORT, &gpio);

    cs_high();
    gpio.Pin = RA8875_CS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Alternate = 0;
    HAL_GPIO_Init(RA8875_CS_PORT, &gpio);

    /* Mode 0, MSB first, as in the vendor's example */
    spi_host.Instance = RA8875_SPI;
    spi_host.Init.Mode = SPI_MODE_MASTER;
    spi_host.Init.Direction = SPI_DIRECTION_2LINES;
    spi_host.Init.DataSize = SPI_DATASIZE_8BIT;
    spi_host.Init.CLKPolarity = SPI_POLARITY_LOW;
    spi_host.Init.CLKPhase = SPI_PHASE_1EDGE;
    spi_host.Init.NSS = SPI_NSS_SOFT;
    spi_host.Init.BaudRatePrescaler = RA8875_SPI_PRESCALER_SLOW;
    spi_host.Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi_host.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;

    return HAL_SPI_Init(&spi_host) == HAL_OK ? 0 : -1;
}

void ra8875_port_set_fast(void)
{
    spi_host.Init.BaudRatePrescaler = RA8875_SPI_PRESCALER_FAST;
    HAL_SPI_Init(&spi_host);
}

void ra8875_port_write_cmd(uint8_t reg)
{
    write_cycle(RA8875_SPI_CMD_WRITE, reg);
}

void ra8875_port_write_data(uint8_t data)
{
    write_cycle(RA8875_SPI_DATA_WRITE, data);
}

void ra8875_port_write_pixels(const uint16_t *pixels, uint32_t count)
{
    uint8_t type = RA8875_SPI_DATA_WRITE;
    uint8_t buf[64];

    /* One data cycle for the whole run: the prefix byte, then the pixels */
    cs_low();
    HAL_SPI_Transmit(&spi_host, &type, 1, RA8875_SPI_TIMEOUT);

    while (count > 0)
    {
        uint32_t n = count < sizeof(buf) / 2 ? count : sizeof(buf) / 2;

        /* RGB565, high byte first */
        for (uint32_t i = 0; i < n; i++)
        {
            buf[2 * i] = (uint8_t)(pixels[i] >> 8);
            buf[2 * i + 1] = (uint8_t)pixels[i];
        }
        HAL_SPI_Transmit(&spi_host, buf, (uint16_t)(2 * n), RA8875_SPI_TIMEOUT);

        pixels += n;
        count -= n;
    }

    cs_high();
}

uint8_t ra8875_port_read_data(void)
{
    return read_cycle(RA8875_SPI_DATA_READ);
}

uint8_t ra8875_port_read_status(void)
{
    return read_cycle(RA8875_SPI_STATUS_READ);
}

int ra8875_port_loopback_test(void)
{
    uint8_t tx[4] = {0xA5, 0x5A, 0x00, 0xFF};
    uint8_t rx[4] = {0};

    /* CS stays high, so an attached RA8875 ignores these bytes */
    if (HAL_SPI_TransmitReceive(&spi_host, tx, rx, sizeof(tx), RA8875_SPI_TIMEOUT) != HAL_OK)
    {
        return -1;
    }

    for (unsigned i = 0; i < sizeof(tx); i++)
    {
        if (rx[i] != tx[i])
        {
            return -1;
        }
    }

    return 0;
}
