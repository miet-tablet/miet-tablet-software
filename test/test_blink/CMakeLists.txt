#

project(test_blink)

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
        firmware_interrupt_handler
        firmware_system_init
)