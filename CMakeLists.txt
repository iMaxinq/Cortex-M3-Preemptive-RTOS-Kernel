cmake_minimum_required(VERSION 3.20)
project(minirtos C ASM)

# create the firmware executable target
add_executable(minirtos.elf
    app/main.c
    startup/startup.c
    drivers/uart.c
    lib/kprintf.c
    lib/ringbuffer.c
    kernel/port/systick.c
    kernel/port/fault.c
    kernel/port/port.c
    kernel/task.c
    kernel/scheduler.c
)

target_include_directories(minirtos.elf PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Compiler flags:
# -mcpu=cortex-m3 : Target the ARM Cortex-M3 processor (used by QEMU mps2-an385)
# -mthumb         : Use the Thumb-2 instruction set (Cortex-M only supports Thumb)
# -g3 -O0         : Maximum debug info and zero optimization so GDB stepping is exact
# -Wall -Wextra -Werror : Turn all warnings into errors
# -ffreestanding  : Tell GCC that standard library functions may not exist
# -fdata-sections -ffunction-sections : Put each function/var in its own sub-section for dead-code removal
target_compile_options(minirtos.elf PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -g3
    -O0
    -Wall
    -Wextra
    -Werror
    -ffreestanding
    -fdata-sections
    -ffunction-sections
)

# Linker flags:
# -T <script>     : Use our custom linker script instead of GCC's default
# -nostartfiles   : Do not use GCC's default C runtime startup (crt0.o); we write our own Reset_Handler
# -nostdlib       : Do not link standard C libraries
# -Wl,--gc-sections : Discard unused functions/variables to save memory
# -Wl,-Map=...    : Output a map file showing exact memory addresses of all symbols
target_link_options(minirtos.elf PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -T ${CMAKE_SOURCE_DIR}/linker/linker.ld
    -nostartfiles
    -nostdlib
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/minirtos.map
)