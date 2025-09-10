cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(main C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Executable and sources
add_executable(main
    main.cpp
    accel.cpp
    MadgwickAHRS.c
    adc.cpp
    servo.cpp
    track.cpp
)

# Optional include directories (headers are in the project root)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Link required SDK libraries
target_link_libraries(main
    pico_stdlib
    hardware_i2c
    hardware_spi
    hardware_pwm
)

# Enable stdio over UART (disable USB)
pico_enable_stdio_uart(main 1)
pico_enable_stdio_usb(main 0)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(main pico_cyw43_arch_none)
endif()

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(main)