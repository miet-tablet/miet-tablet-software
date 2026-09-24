cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)
set(FIRMWARE_LOCATION FLASH)
# set(FIRMWARE_LOCATION RAM)
set(MCU STM32H7B0)
# set(MCU STM32H745)
# set(LDSCRIPT_NAME ${MCU}_${FIRMWARE_LOCATION}.ld)
# set(LDSCRIPT_PATH Core/linkerscript/${LDSCRIPT_NAME})
set(LDSCRIPT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/linkerscript/stm32h7b0_flash.ld)
set(STARTUP_NAME ${MCU}_startup.S)
set(STARTUP_PATH Core/startup/${STARTUP_NAME})

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(main C ASM)

set(ARCH_FLAGS
    -mcpu=cortex-m7 
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
    -nostdlib
    --specs=nano.specs
    --specs=nosys.specs
    -O2
    -D${MCU}xx
    -DCORE_CM7
    # "--include=stdint.h"
)
set(W_FLAGS
    -Wall
	-Wextra
	-Wduplicated-branches
	-Wduplicated-cond
	-Wfloat-equal
	-Wlogical-op
	-Wsign-conversion
	-Wrestrict
	-Wno-comment
	-Wno-comment
	-Wno-unused-parameter
	-Wno-sign-conversion
)
set(F_FLAGS
    -ffunction-sections
)

add_compile_options(${TARGET}
    ${ARCH_FLAGS}
    ${W_FLAGS}
    ${F_FLAGS}
)

include_directories(conf)

set(TARGET main)

# add_subdirectory(Core/stm32h7xx_hal)
# add_subdirectory(examples/weact_stm32h7b0_blink)

add_link_options(
    #${ARCH_FLAGS}
    -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -nostdlib -Wl,--gc-sections
    "-T${LDSCRIPT_PATH}"
    #"-Wl,--gc-sections"
    #"-nostartfiles"
    #"-Wl,-Map=${PROJECT_NAME}.map"
    #-fuse-ld=lld
)

# add_executable(main.elf ${STARTUP_PATH})

# target_link_libraries(main.elf stm32h7xx_hal weact_stm32h7b0_blink)

add_subdirectory(STM32CubeH7)
add_subdirectory(firmware/interrupt_handler)
add_subdirectory(firmware/system_init)
add_subdirectory(test/test_blink)

# add_executable(main.elf STM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7b0xx.s)
add_executable(main.elf startup/stm32h7b0_startup.S)

target_link_libraries(main.elf ${TOP} "-Wl,--whole-archive" firmware_interrupt_handler "-Wl,--no-whole-archive")

add_custom_command(
    TARGET main.elf
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary main.elf main.bin
    COMMAND ${CMAKE_OBJDUMP} ARGS -S main.elf > main.lst
)