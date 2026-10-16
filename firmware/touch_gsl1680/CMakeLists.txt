project(firmware_touch_gsl1680)

add_library(${PROJECT_NAME} STATIC
    src/touch_gsl1680.c
    src/touch_gsl1680_firmware.c
)

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CMSIS_Core
        STM32H7xx
        STM32H7xx_HAL_Driver
)
