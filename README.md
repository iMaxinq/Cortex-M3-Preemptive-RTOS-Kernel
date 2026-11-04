# Cortex-M3 Preemptive RTOS Kernel

A small preemptive real-time kernel for the ARM Cortex-M3, written from scratch in C with no vendor libraries, no C library and no heap. It runs on QEMU's `mps2-an385` board.

It covers everything from the reset vector up: startup code, an interrupt-driven UART driver, `printf`-style output, a 1 ms SysTick timer, context switching through PendSV, and a priority scheduler with sleeping tasks and an idle task. Host-side unit tests cover the logic that doesn't depend on hardware.

The firmware is about 4 KB of code.

## Features

- **Preemptive priority scheduling.** The highest-priority ready task always runs. Tasks of equal priority share the CPU round-robin, 1 ms each.
- **Immediate preemption.** A high-priority task takes over on the same tick it wakes up.
- **`task_sleep(ms)`.** Sleeping tasks are never scheduled, so they use no CPU time.
- **Idle task.** It halts the core with `wfi` when nothing is ready, and counts idle ticks so you can measure CPU load.
- **Context switching** in PendSV, at the lowest exception priority, with each task on its own stack through PSP.
- **Bare-metal startup.** A hand-written vector table, the `.data` copy and `.bss` zeroing, and a custom linker script.
- **UART driver.** Polled TX, and an interrupt-driven RX into a lock-free ring buffer.
- **`kprintf`.** A minimal `printf` supporting `%c %s %d %u %x %p %%`.
- **HardFault reporter.** It prints the stacked registers plus CFSR and HFSR when the CPU faults.
- **Unit tests.** They use [Unity](https://github.com/ThrowTheSwitch/Unity) and run on the host for the scheduler, the ring buffer and `kprintf`.

## Demo

`app/main.c` runs three tasks:

| Task | Priority | Behaviour |
| :--- | :---: | :--- |
| HIGH | 3 | Wakes every 300 ms and reports how late it was |
| MON | 2 | Reports how many of the last 1000 ticks were idle |
| LOW | 1 | Busy for 200 ms, then sleeps 300 ms (~40% CPU load) |

The output shows that HIGH is never late, even when it wakes in the middle of LOW's work. It also shows that the CPU is idle for the remaining 60% of the time, which is how you can tell sleeping tasks use no CPU:

```
[LOW ] finished work round 0
[HIGH] woke at tick 300, late by 0 ticks
[HIGH] woke at tick 600, late by 0 ticks (preempted LOW mid-work)
[LOW ] finished work round 1
[HIGH] woke at tick 900, late by 0 ticks
[MON ] idle 600 of last 1000 ticks
```

## Requirements

| Tool | Tested version |
| :--- | :--- |
| `arm-none-eabi-gcc` | 10.3 |
| `qemu-system-arm` | 6.2 |
| CMake + Ninja | 3.22 |
| Host `gcc` (unit tests) | any C11 compiler |
| `gdb-multiarch` (optional) | — |

On Ubuntu/Debian:

```sh
sudo apt install gcc-arm-none-eabi qemu-system-arm cmake ninja-build gcc gdb-multiarch
```

## Build and run

```sh
# Configure with the ARM toolchain and build build/minirtos.elf (warnings are errors)
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
cmake --build build

# Run in QEMU; UART0 is connected to the terminal. Quit with Ctrl-A then X.
qemu-system-arm -M mps2-an385 -nographic -kernel build/minirtos.elf
```

## Debug with GDB

```sh
# Start QEMU paused (-S) with a GDB server on port 1234 (-s)
qemu-system-arm -M mps2-an385 -nographic -kernel build/minirtos.elf -s -S

# In a second terminal
gdb-multiarch build/minirtos.elf -ex "target remote :1234"
```

The build uses `-g3 -O0`, so stepping matches the source line by line. A linker map is written to `build/minirtos.map`.

## Unit tests

The tests build natively on the host. They don't need the ARM toolchain or QEMU.

```sh
cmake -S tests/unit -B build-tests -G Ninja
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

| Suite | Covers |
| :--- | :--- |
| `test_sched` | Picking the highest-priority task, round-robin, skipping blocked tasks, falling back to idle, waking sleepers, tick wrap-around, and preemption on wake |
| `test_ringbuffer` | Empty, full and wrap-around behaviour |
| `test_kprintf` | Every format specifier, including `INT_MIN` and `NULL` strings |

## Project layout

```
app/            Demo application (main.c)
startup/        Vector table and Reset_Handler
linker/         Linker script (FLASH/RAM layout)
drivers/        UART0 driver
lib/            kprintf and ring buffer
kernel/         Task creation, sleeping, idle task, scheduling policy
kernel/port/    Cortex-M3 specific code: PendSV context switch, SysTick, HardFault
include/        Public headers
tests/unit/     Host unit tests
tests/unity/    Vendored Unity test framework
cmake/          ARM cross-compilation toolchain file
```

## API

```c
#include "minirtos.h"

int      task_create(task_func_t entry, const char *name, uint8_t priority);
void     task_sleep(uint32_t ms);       /* task_sleep(0) yields */
uint32_t task_get_idle_ticks(void);
void     rtos_start(void);              /* never returns */
```

Higher numbers mean higher priority. Priority 0 is reserved for the idle task. Up to 7 application tasks can be created (`MAX_TASKS` is 8, including idle), each with a 1 KB stack.

## How it works

**Boot.** The CPU loads the initial stack pointer and `Reset_Handler` from the vector table at `0x00000000`. `Reset_Handler` copies `.data` from FLASH to RAM, zeroes `.bss`, and calls `main()`.

**Task creation.** `task_create()` builds a fake exception frame at the top of the new task's stack. The first context switch then "returns" into the task's entry function as if the task had been interrupted.

**Starting.** `rtos_start()` adds the idle task and selects the highest-priority task. It then sets PSP to 0 to mean "no outgoing task", resets MSP for interrupt handlers, and triggers PendSV.

**Every 1 ms tick.** `SysTick_Handler` wakes any sleeping task whose time is up, then asks the scheduler for the highest-priority ready task. If that task isn't the one running, it triggers PendSV.

**Context switch.** `PendSV_Handler` saves R4–R11 onto the outgoing task's stack and stores its stack pointer in the task record. It then loads the incoming task's stack pointer and restores R4–R11. The hardware restores everything else on exception return.

**Scheduling policy.** The policy lives in `kernel/scheduler.c` and doesn't touch hardware, which is why it can be unit-tested on the host.

## Limitations

- Single core, Cortex-M3 only. The context switch doesn't save the FPU state of an M4F or M7.
- Tasks are created statically and can't be deleted.
- Nothing protects data shared between tasks. There are no mutexes, semaphores or queues, so any task can be preempted in the middle of `kprintf`.
- There's no stack overflow detection.

## Hardware reference (QEMU mps2-an385)

### Memory map

```
0x00000000 - 0x1FFFFFFF  FLASH       code and constants
0x20000000 - 0x3FFFFFFF  SRAM        variables and stacks
0x40000000 - 0x5FFFFFFF  Peripherals UART0 at 0x40004000
0xE0000000 - 0xFFFFFFFF  System      SysTick at 0xE000E010, NVIC at 0xE000E100, SCB at 0xE000ED00
```

### UART0 registers (CMSDK UART, base `0x40004000`)

| Register | Offset | Access | Behaviour |
| :--- | :--- | :--- | :--- |
| **DATA** | `+0x00` | R/W | **Bits [7:0].** A write goes to the TX holding buffer. A read returns the byte in the RX holding buffer. |
| **STATE** | `+0x04` | R/W | **Bit 0 TX_FULL:** TX is busy, and a write to DATA now would be lost.<br>**Bit 1 RX_FULL:** a received byte is waiting in DATA.<br>**Bit 2 TX_OVERRUN:** DATA was written while TX_FULL was set.<br>**Bit 3 RX_OVERRUN:** a byte arrived before the previous one was read. |
| **CTRL** | `+0x08` | R/W | **Bit 0 TX_EN**, **Bit 1 RX_EN:** enable the transmitter and receiver.<br>**Bit 2 TX_IRQ_EN**, **Bit 3 RX_IRQ_EN:** enable the TX and RX interrupts. |
| **INTCLEAR** | `+0x0C` | W | **Bit 0:** clears the TX interrupt.<br>**Bit 1:** clears the RX interrupt. `UART0RX_Handler` must write it, or the interrupt fires again forever. |
| **BAUDDIV** | `+0x10` | R/W | **Bits [19:0]:** baud rate divider. 25 MHz / 217 ≈ 115200 baud. |
