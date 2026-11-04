cmake_minimum_required(VERSION 3.20)
project(minirtos C ASM)

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

# -mcpu=cortex-m3 -mthumb : Cortex-M3 (QEMU mps2-an385) runs Thumb-2 only
# -g3 -O0                 : full debug info, no optimization, exact GDB stepping
# -Wall -Wextra -Werror   : every warning is an error
# -ffreestanding          : no hosted C library
# -f*-sections            : one section per symbol so --gc-sections can drop unused ones
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

# -T linker.ld            : our memory layout
# -nostartfiles -nostdlib : no crt0 or libc; startup.c provides Reset_Handler
# --gc-sections           : drop unused sections
# -Map                    : write a map of every symbol's address
target_link_options(minirtos.elf PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -T ${CMAKE_SOURCE_DIR}/linker/linker.ld
    -nostartfiles
    -nostdlib
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/minirtos.map
)