# https://github.com/queezythegreat/arduino-cmake

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

include(ArduinoToolchain)
include(Arduino)

print_board_list()
print_programmer_list()

generate_arduino_firmware(demo_arduino_board
        SRCS main.cpp
        PORT  /dev/ttyACM0
        BOARD uno)