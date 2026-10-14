#include "i2c_wheels.h"

static I2C_HandleTypeDef *i2c_host;

void i2c_wheels_set_i2c_host(I2C_HandleTypeDef *host)
{
    i2c_host = host;
}

int i2c_wheels_read(uint8_t *value, int len)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Receive(
        i2c_host,
        0,
        value,
        len,
        HAL_MAX_DELAY
    );

    return status == HAL_OK ? 0 : -1;
}

int i2c_wheels_write(const uint8_t *value, int len)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Transmit(
        i2c_host,
        0,
        (uint8_t *)value,
        len,
        HAL_MAX_DELAY
    );

    return status == HAL_OK ? 0 : -1;
}