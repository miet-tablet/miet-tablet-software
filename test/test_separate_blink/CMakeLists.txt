#

project(firmware-main)

set(SOURCES
    src/main.c
    src/system_stm32h7b0xx.c
    src/stm32h7xx_it.c
    src/stm32h7xx_hal_msp.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CMSIS_Core
        STM32H7xx
        STM32H7xx_HAL_Driver
)
