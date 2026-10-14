project(test_adc_wheels)

set(SOURCES
    src/main.c
    src/system_stm32h7b0xx.c
    src/stm32h7xx_hal_msp.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC inc)

add_library(hal_adc STATIC
    ${CMAKE_SOURCE_DIR}/STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c
    ${CMAKE_SOURCE_DIR}/STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c
)

target_compile_definitions(hal_adc PRIVATE HAL_ADC_MODULE_ENABLED)

target_link_libraries(hal_adc
    PUBLIC
        STM32H7xx
        STM32H7xx_HAL_Driver
        CMSIS_Core
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CMSIS_Core
        STM32H7xx
        STM32H7xx_HAL_Driver
        firmware_adc_wheels
        hal_adc
)
