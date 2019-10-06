
#TODO: Also use geda-gaf-1.9.2 source code to get gaf-export for dynamic circuit drawings

# resource files used by binary target are copied in to the binary build folder
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rc.cir
               ${CMAKE_CURRENT_BINARY_DIR}/rc.cir COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/spinit
               ${CMAKE_CURRENT_BINARY_DIR}/../share/ngspice/scripts/spinit COPYONLY)


file(GLOB TEST_SRC
        "*.cpp")


# First look for a pre-compiled and installed binary of NgSpice using find_package.
# However usually this is a special case, therefore if none is found
# try to fetch NgSpice sources and build them within cmake-build-debug.
# These locally compiled ngspice will be used directly without installing it
find_package(NgSpice)
if (NGSPICE_LIBRARY MATCHES "NOTFOUND")

    message(STATUS "NgSpice not found, trying to fetch and build it locally")

    SET(NGSPICE_ROOT "${CMAKE_BINARY_DIR}/external/ngspice")

    include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
    ExternalProject_Add(lib_ngspice
            PREFIX "${NGSPICE_ROOT}"
            URL "https://sourceforge.net/projects/ngspice/files/ng-spice-rework/30/ngspice-30.tar.gz/download"
            CONFIGURE_COMMAND ${NGSPICE_ROOT}/src/lib_ngspice/configure
                                --with-ngshared
                                --enable-relpath # search for spinit in the ../share/ngspice/scripts
                                --disable-maintainer-mode
                                --disable-openmp
                                --without-wingui
                                --without-x
            INSTALL_COMMAND "")

    # print directories
    ExternalProject_Get_Property(lib_ngspice BINARY_DIR)
    ExternalProject_Get_Property(lib_ngspice SOURCE_DIR)
    message(STATUS "BINARY_DIR = ${BINARY_DIR}")
    message(STATUS "SOURCE_DIR = ${SOURCE_DIR}")

    # configure target for NgSpice library
    add_library(lib_ngspice_prj SHARED IMPORTED)
    set_target_properties(lib_ngspice_prj
            PROPERTIES IMPORTED_LOCATION
            ${BINARY_DIR}/src/.libs/${CMAKE_SHARED_LIBRARY_PREFIX}ngspice${CMAKE_SHARED_LIBRARY_SUFFIX})
    include_directories(${SOURCE_DIR}/src/include/)

    # point binaries to the locally compiled NgSpice
    add_executable (demo_ngspice ${TEST_SRC})
    target_link_libraries(demo_ngspice lib_ngspice_prj)
    target_link_libraries(demo_ngspice ${CMAKE_THREAD_LIBS_INIT})
    add_dependencies(demo_ngspice lib_ngspice)
    set_property(TARGET demo_ngspice PROPERTY CXX_STANDARD 11)

else()

    # point binaries on to the globally installed NgSpice
    add_executable (demo_ngspice ${TEST_SRC})
    target_link_libraries(demo_ngspice ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(demo_ngspice ${NGSPICE_LIBRARY})
    set_property(TARGET demo_ngspice PROPERTY CXX_STANDARD 11)

endif()