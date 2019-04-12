#find_package(esptool)

# Tool to burn esp8266 firmware image via FTDI UART
find_program(ESPTOOL esptool)

# Native ESP8266 Non-OS SDK by Espressif Systems
# https://www.espressif.com/sites/default/files/documentation/
#         2c-esp8266_non_os_sdk_api_reference_en.pdf

# Downloading it from the Github
# https://github.com/espressif/ESP8266_NONOS_SDK

message(STATUS "Trying to fetch and build ESP8266 NonOS SDK locally")

SET(ESP8266_NONOS_SDK "${CMAKE_BINARY_DIR}/external/esp8266_sdk")

# Download from github
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
ExternalProject_Add(lib_esp8266_sdk
        PREFIX "${ESP8266_NONOS_SDK}"
        GIT_REPOSITORY "https://github.com/espressif/ESP8266_NONOS_SDK"
        BUILD_COMMAND "make"
        CONFIGURE_COMMAND ""
        INSTALL_COMMAND ""
        GIT_PROGRESS 1
        BUILD_IN_SOURCE 1)

# SDK Source folder to compile into
ExternalProject_Get_Property(lib_esp8266_sdk SOURCE_DIR)
message(STATUS "ESP8266 Source: ${SOURCE_DIR}")

file(GLOB FIRMWARE_SRC
        "*.cpp")

# link library to the firmware main file
add_executable (demo_esp8266_firmware ${FIRMWARE_SRC})
target_include_directories(demo_esp8266_firmware ${})
set_property(TARGET demo_esp8266_firmware PROPERTY CXX_STANDARD 11)