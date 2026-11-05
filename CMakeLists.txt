cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico2_firmware C CXX ASM)

pico_sdk_init()

add_executable(pico2_firmware
    main.c
)

target_link_libraries(pico2_firmware
    pico_stdlib
)

pico_enable_stdio_usb(pico2_firmware 1)
pico_enable_stdio_uart(pico2_firmware 0)

pico_add_extra_outputs(pico2_firmware)
