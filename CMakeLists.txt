cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(remote-pi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
pico_sdk_init()

add_executable(remote-pi 
  src/main.c
  src/ir/nec.c
  src/buttons.c
  src/state.c
  src/remote_sleep.c
)

pico_generate_pio_header(remote-pi 
${CMAKE_CURRENT_LIST_DIR}/src/ir/nec_burst_control.pio)
pico_generate_pio_header(remote-pi
${CMAKE_CURRENT_LIST_DIR}/src/ir/nec_carrier_control.pio)

target_link_libraries(remote-pi 
  pico_stdlib 
  hardware_i2c
  hardware_gpio
  hardware_pio 
  hardware_clocks
  hardware_adc 
  pico_low_power
)
# Enable USB CDC serial, disable UART pins
pico_enable_stdio_usb(remote-pi 1)
pico_enable_stdio_uart(remote-pi 0)

pico_add_extra_outputs(remote-pi)