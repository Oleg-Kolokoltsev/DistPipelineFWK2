cmake_minimum_required (VERSION 3.1)

project (DistPipelineFWK)

set (DistPipelineFWK_VERSION_MAJOR 1)
set (DistPipelineFWK_VERSION_MINOR 0)

# setup path to the custom CMake modules and scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

# check main packages and fill up variables for configure.h
include(PlatformIntrospection)

# automatic configure file template 'config.h.in' stored in the project root folder
# will be processed into the valid header in binary folder 'config.h'
# the file config.h is produced automatically and shell not be included in
# version control
configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

# include binary folder into the search path for all targets
include_directories("${PROJECT_BINARY_DIR}")

# in all code the benefits of C11 standard are used, so it is
# required for all project
set_property(GLOBAL PROPERTY CXX_STANDARD 11)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED true)

# follw the src folder tree to locate the rest of CMakeLists.txt scripts
add_subdirectory(src)