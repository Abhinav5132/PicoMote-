cmake_minimum_required(VERSION 3.13)

# Target Raspberry Pi Pico 2 (RP2350) - MUST be set before including SDK!
set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(remote-pi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(remote-pi 
  main.c
  nec.c
)

target_link_libraries(remote-pi 
  pico_stdlib 
  hardware_i2c
  hardware_gpio
  hardware_pwm
  hardware_clocks
)

# Enable USB CDC serial, disable UART pins
pico_enable_stdio_usb(remote-pi 1)
pico_enable_stdio_uart(remote-pi 0)

pico_add_extra_outputs(remote-pi)