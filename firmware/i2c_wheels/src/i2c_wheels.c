#include "i2c_wheels.h"
static I2C_HandleTypeDef *i2c_host;
static uint8_t device_address = 0;
void i2c_wheels_set_i2c_host(I2C_HandleTypeDef *host)
{
    i2c_host = host;
}
void i2c_wheels_set_i2c_device_address(uint8_t address)
{
    device_address = address;
}
int i2c_wheels_read(uint8_t address, uint8_t *value, int len)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Read
    (
        i2c_host,
        device_address,
        address,
        I2C_MEMADD_SIZE_8BIT,
        value,
        len,
        HAL_MAX_DELAY
    );
    return status == HAL_OK ? 0 : -1;
}
int i2c_wheels_write(uint8_t address, uint8_t *value, int len)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Write
    (
        i2c_host,
        device_address,
        address,
        I2C_MEMADD_SIZE_8BIT,
        value,
        len,
        HAL_MAX_DELAY
    );
    return status == HAL_OK ? 0 : -1;
}
