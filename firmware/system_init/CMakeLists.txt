#

project(firmware_system_init)

set(SOURCES
    src/clock_config.c
    src/error_handler.c
    src/mpu_config.c
    src/stm32h7xx_hal_msp.c
    src/system_stm32h7b0xx.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CMSIS_Core
        STM32H7xx
        STM32H7xx_HAL_Driver
)
