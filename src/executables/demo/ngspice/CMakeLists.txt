#TODO: https://cmake.org/cmake/help/latest/module/ExternalProject.html#examples
#TODO: Also use geda-gaf-1.9.2 source code to get gaf-export for dynamic circuit drawings

# To install ngspice shared library:
# download and unpack the latest version of ngspice from
# https://sourceforge.net/projects/ngspice/files/ng-spice-rework/
#
# > ./configure  --with-ngshared --enable-relpath
# > make
# > sudo make install
#
# The library and it's includes will appear in the /usr/local prefix

find_package(NgSpice REQUIRED)

message(STATUS "NGSPICE_INCLUDE_DIR = ${NGSPICE_INCLUDE_DIR}")
message(STATUS "NGSPICE_LIBRARY = ${NGSPICE_LIBRARY}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rc.cir ${CMAKE_CURRENT_BINARY_DIR}/rc.cir COPYONLY)

add_executable (demo_ngspice main.cpp)
target_link_libraries(demo_ngspice ${NGSPICE_LIBRARY})
set_property(TARGET demo_ngspice PROPERTY CXX_STANDARD 11)