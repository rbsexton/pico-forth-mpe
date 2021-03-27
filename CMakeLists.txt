cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(pico-forth-mpe_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
add_executable(uart_advanced
    uart_advanced.c
    ringbuffer.c
)


pico_add_extra_outputs(uart_advanced)

target_link_libraries(uart_advanced pico_stdlib)
