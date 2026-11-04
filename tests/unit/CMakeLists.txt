cmake_minimum_required(VERSION 3.16)
project(miniRTOS_unit_tests C)

enable_testing()
add_compile_options(-Wall -Wextra -Werror -std=c11)

# Vendored Unity (v2.6.0)
add_library(unity STATIC ../unity/unity.c)
target_include_directories(unity PUBLIC ../unity)

add_executable(test_ringbuffer
    test_ringbuffer.c
    ../../lib/ringbuffer.c
)
target_include_directories(test_ringbuffer PRIVATE ../../include)
target_link_libraries(test_ringbuffer PRIVATE unity)
add_test(NAME ringbuffer_test COMMAND test_ringbuffer)

add_executable(test_kprintf
    test_kprintf.c
    ../../lib/kprintf.c
)
target_include_directories(test_kprintf PRIVATE ../../include)
target_link_libraries(test_kprintf PRIVATE unity)
add_test(NAME kprintf_test COMMAND test_kprintf)

add_executable(test_sched
    test_sched.c
    ../../kernel/scheduler.c
)
target_include_directories(test_sched PRIVATE ../../include)
target_link_libraries(test_sched PRIVATE unity)
add_test(NAME sched_test COMMAND test_sched)
