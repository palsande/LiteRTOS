cmake_minimum_required(VERSION 3.10)
project(rtos-framework LANGUAGES C CXX ASM)

# Set cross-compiler
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Ensure we use ARM GCC
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)

# CPU flags for Cortex-M4
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wall -ffreestanding -nostdlib -g")

set(CMAKE_C_FLAGS "${CPU_FLAGS}")
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} -fno-rtti -fno-exceptions -std=c++17")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/hal
    ${CMAKE_SOURCE_DIR}/platform/tm4c123gxl
    ${CMAKE_SOURCE_DIR}/platform/tm4c123gxl_bsp/tivaware_c_series_2_1_4_178/inc
)

# Source files
set(SOURCES
    kernel/rtos.cpp
    hal/port.cpp
    examples/main.cpp
    platform/tm4c123gxl/startup.s
)

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/platform/tm4c123gxl/linker.ld)

# Create executable
add_executable(rtos-framework
    kernel/rtos.cpp
    hal/port.cpp
    examples/main.cpp
    platform/tm4c123gxl/startup.s
)

# Set properties for ELF file
set_target_properties(rtos-framework PROPERTIES OUTPUT_NAME "rtos-framework" SUFFIX ".elf")

# Linker options
target_link_options(rtos-framework PRIVATE
    -T${LINKER_SCRIPT}   # Use custom linker script
    -nostartfiles
    -Wl,--gc-sections
)

# Convert ELF to BIN
add_custom_command(TARGET rtos-framework POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary rtos-framework.elf rtos-framework.bin
    COMMENT "Generated rtos-framework.elf and rtos-framework.bin"
)
