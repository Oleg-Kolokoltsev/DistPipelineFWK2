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
#
# Strategy: first look for a pre-compiled binary on the system and only
# if none is found, fetch the sources and build
# Source: https://foonathan.net/blog/2016/07/07/cmake-dependency-handling.html

find_package(NgSpice)
if (NGSPICE_LIBRARY MATCHES "NOTFOUND")
    message(STATUS "NgSpice dependency not found, trying to fetch and build it locally")

    SET(LOCAL_NGSPICE_ROOT "${CMAKE_BINARY_DIR}/external/ngspice")

    include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
    ExternalProject_Add(LocalNgSpice
            PREFIX "${LOCAL_NGSPICE_ROOT}"
            URL "https://sourceforge.net/projects/ngspice/files/ng-spice-rework/30/ngspice-30.tar.gz/download"
            #DOWNLOAD_DIR ${LOCAL_NGSPICE_ROOT}
            #SOURCE_DIR "${LOCAL_NGSPICE_ROOT}/src"
            #BUILD_DIR "${LOCAL_NGSPICE_ROOT}/build"
            #DOWNLOAD_COMMAND "wget"
            CONFIGURE_COMMAND ./configure --with-ngshared --enable-relpath
            BUILD_COMMAND  "make"
            INSTALL_COMMAND ""
            BUILD_IN_SOURCE 1
            LOG_BUILD 1)
endif()

message(STATUS "NGSPICE_INCLUDE_DIR = ${NGSPICE_INCLUDE_DIR}")
message(STATUS "NGSPICE_LIBRARY = ${NGSPICE_LIBRARY}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rc.cir ${CMAKE_CURRENT_BINARY_DIR}/rc.cir COPYONLY)

#add_executable (demo_ngspice main.cpp)
#target_link_libraries(demo_ngspice ${NGSPICE_LIBRARY})
#set_property(TARGET demo_ngspice PROPERTY CXX_STANDARD 11)