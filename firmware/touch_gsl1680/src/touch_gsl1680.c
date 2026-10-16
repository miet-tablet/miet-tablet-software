#include "touch_gsl1680.h"

#define GSL_DATA_REG    0x80U
#define GSL_STATUS_REG  0xE0U
#define GSL_TIMEOUT_MS  100U
#define GSL_MAX_TOUCHES 10U

static int write_reg(gsl1680_t *dev, uint8_t reg, const uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Write(dev->i2c, GSL1680_I2C_ADDR, reg,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t *)data,
                             len, GSL_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

static int read_reg(gsl1680_t *dev, uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(dev->i2c, GSL1680_I2C_ADDR, reg,
                            I2C_MEMADD_SIZE_8BIT, data,
                            len, GSL_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

static int write_u8(gsl1680_t *dev, uint8_t reg, uint8_t value)
{
    return write_reg(dev, reg, &value, 1);
}

static int reset_chip(gsl1680_t *dev)
{
    const uint8_t zeros[4] = {0};

    if (write_u8(dev, 0xE0, 0x88) != 0) return -1;
    HAL_Delay(10);
    if (write_u8(dev, 0xE4, 0x04) != 0) return -1;
    HAL_Delay(10);
    if (write_reg(dev, 0xBC, zeros, sizeof(zeros)) != 0) return -1;
    HAL_Delay(10);
    return 0;
}

static int clear_registers(gsl1680_t *dev)
{
    if (write_u8(dev, 0xE0, 0x88) != 0) return -1;
    HAL_Delay(10);
    if (write_u8(dev, 0x80, 0x01) != 0) return -1;
    HAL_Delay(5);
    if (write_u8(dev, 0xE4, 0x04) != 0) return -1;
    HAL_Delay(5);
    if (write_u8(dev, 0xE0, 0x00) != 0) return -1;
    HAL_Delay(10);
    return 0;
}

static int load_firmware(gsl1680_t *dev, const gsl1680_fw_entry_t *fw, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint32_t v = fw[i].value;
        uint8_t bytes[4] = {
            (uint8_t)v,
            (uint8_t)(v >> 8),
            (uint8_t)(v >> 16),
            (uint8_t)(v >> 24),
        };

        /* GSL firmware: 0xF0 selects a page, 0x00..0x7C holds words. */
        if (fw[i].reg != 0xF0U && (fw[i].reg > 0x7CU || (fw[i].reg & 3U) != 0U))
        {
            return -1;
        }
        uint16_t len = fw[i].reg == 0xF0U ? 1U : (uint16_t)sizeof(bytes);
        if (write_reg(dev, fw[i].reg, bytes, len) != 0) return -1;
    }
    return 0;
}

int gsl1680_init(gsl1680_t *dev, const gsl1680_fw_entry_t *fw, size_t fw_count)
{
    if (dev == NULL || dev->i2c == NULL) return -1;
    dev->ready = false;

    if (dev->reset_port != NULL)
    {
        HAL_GPIO_WritePin(dev->reset_port, dev->reset_pin, GPIO_PIN_RESET);
        HAL_Delay(20);
        HAL_GPIO_WritePin(dev->reset_port, dev->reset_pin, GPIO_PIN_SET);
        HAL_Delay(20);
    }

    if (HAL_I2C_IsDeviceReady(dev->i2c, GSL1680_I2C_ADDR, 3, GSL_TIMEOUT_MS) != HAL_OK)
    {
        return -2;
    }
    if (fw == NULL || fw_count == 0) return -3;

    if (clear_registers(dev) != 0 || reset_chip(dev) != 0) return -4;
    if (load_firmware(dev, fw, fw_count) != 0) return -5;

    /* Start, reset once again, then start again: GSL1680 reference sequence. */
    if (write_u8(dev, GSL_STATUS_REG, 0x00) != 0) return -6;
    HAL_Delay(10);
    if (reset_chip(dev) != 0) return -6;
    if (write_u8(dev, GSL_STATUS_REG, 0x00) != 0) return -6;
    HAL_Delay(30);

    uint8_t signature[4];
    if (read_reg(dev, 0xB0, signature, sizeof(signature)) != 0) return -7;
    for (size_t i = 0; i < sizeof(signature); ++i)
    {
        if (signature[i] != 0x5A) return -7;
    }

    dev->ready = true;
    return 0;
}

int gsl1680_read(gsl1680_t *dev, gsl1680_touch_t *touch)
{
    if (dev == NULL || touch == NULL || !dev->ready) return -1;

    /* 0x80: contact count; 0x84..0x87: first finger X,Y and ID. */
    uint8_t data[8] = {0};
    if (read_reg(dev, GSL_DATA_REG, data, sizeof(data)) != 0) return -2;
    if (data[0] > GSL_MAX_TOUCHES) return -3;

    touch->contacts = data[0];
    touch->pressed = data[0] > 0;
    touch->x = 0;
    touch->y = 0;
    touch->finger_id = 0;
    if (!touch->pressed) return 0;

    touch->x = (uint16_t)(data[4] | ((uint16_t)(data[5] & 0x0F) << 8));
    touch->y = (uint16_t)(data[6] | ((uint16_t)(data[7] & 0x0F) << 8));
    touch->finger_id = data[7] >> 4;
    return 0;
}
