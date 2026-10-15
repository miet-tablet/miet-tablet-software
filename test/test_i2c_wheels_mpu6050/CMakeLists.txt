#

project(test_i2c_wheels_mpu6050)

set(SOURCES
    src/main.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CMSIS_Core
        STM32H7xx
        STM32H7xx_HAL_Driver
        firmware_i2c_wheels
        firmware_system_init
        firmware_stdio_uart
)
