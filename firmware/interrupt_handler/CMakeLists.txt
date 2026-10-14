#

project(firmware_interrupt_handler)

set(SOURCES
    src/stm32h7xx_it.c
)

# OBJECT, not STATIC: the linker never pulls archive members just to override
# the weak handlers from startup, so SysTick_Handler would stay a dead loop
add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CMSIS_Core
        STM32H7xx
        STM32H7xx_HAL_Driver
)
