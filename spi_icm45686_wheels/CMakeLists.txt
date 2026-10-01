project(spi_icm45686_wheels)

set(SOURCES
    src/spi_icm45686_wheels.c
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CMSIS_Core
        STM32H7xx
        STM32H7xx_HAL_Driver
)