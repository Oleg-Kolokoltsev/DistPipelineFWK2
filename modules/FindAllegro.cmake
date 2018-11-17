
# - Find ALLEGRO
# Find the ALLEGRO includes and libraries
#
#  ALLEGRO_INCLUDE_DIR - where to find ALLEGRO headers.
#  ALLEGRO_LIBRARIES   - List of libraries when using Allegro.
#  ALLEGRO_FOUND       - True if libOgreMain found.

function(append_lib_type_suffix var)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
    if(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug")
        set(${var} "${${var}}-debug" PARENT_SCOPE)
    endif(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug")
    if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "profile")
        set(${var} "${${var}}-profile" PARENT_SCOPE)
    endif(CMAKE_BUILD_TYPE_TOLOWER MATCHES "profile")
endfunction(append_lib_type_suffix)

if(ALLEGRO_INCLUDE_DIR)
    # Already in cache, be silent
    set(ALLEGRO_FIND_QUIETLY TRUE)
endif(ALLEGRO_INCLUDE_DIR)

find_path(ALLEGRO_INCLUDE_DIR allegro5/allegro.h)

set (ALLEGRO_NAME allegro)
set (ALLEGRO_MAIN_NAME allegro_main)
set (ALLEGRO_IMAGE_NAME allegro_image)
set (ALLEGRO_FONT_NAME allegro_font)
set (ALLEGRO_TTF_NAME allegro_ttf)
set (ALLEGRO_PRIMITIVES_NAME allegro_primitives)
set (ALLEGRO_MEMFILE_NAME allegro_memfile)

append_lib_type_suffix(ALLEGRO_NAME)
append_lib_type_suffix(ALLEGRO_MAIN_NAME)
append_lib_type_suffix(ALLEGRO_IMAGE_NAME)
append_lib_type_suffix(ALLEGRO_FONT_NAME)
append_lib_type_suffix(ALLEGRO_TTF_NAME)
append_lib_type_suffix(ALLEGRO_PRIMITIVES_NAME)
append_lib_type_suffix(ALLEGRO_MEMFILE_NAME)

find_library(ALLEGRO_LIBRARY NAMES ${ALLEGRO_NAME} allegro)
find_library(ALLEGRO_MAIN_LIBRARY NAMES ${ALLEGRO_MAIN_NAME} allegro_main)
find_library(ALLEGRO_IMAGE_LIBRARY NAMES ${ALLEGRO_IMAGE_NAME} allegro_image)
find_library(ALLEGRO_FONT_LIBRARY NAMES ${ALLEGRO_FONT_NAME} allegro_font)
find_library(ALLEGRO_TTF_LIBRARY NAMES ${ALLEGRO_TTF_NAME} allegro_ttf)
find_library(ALLEGRO_PRIMITIVES_LIBRARY NAMES ${ALLEGRO_PRIMITIVES_NAME} allegro_primitives)
find_library(ALLEGRO_MEMFILE_LIBRARY NAMES ${ALLEGRO_MEMFILE_NAME} allegro_memfile)

# Handle the QUIETLY and REQUIRED arguments and set ALLEGRO_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ALLEGRO DEFAULT_MSG
    ALLEGRO_INCLUDE_DIR
    ALLEGRO_LIBRARY
    ALLEGRO_MAIN_LIBRARY
    ALLEGRO_IMAGE_LIBRARY
    ALLEGRO_FONT_LIBRARY
    ALLEGRO_TTF_LIBRARY
    ALLEGRO_PRIMITIVES_LIBRARY
    ALLEGRO_MEMFILE_LIBRARY)

if(ALLEGRO_FOUND)
    list(APPEND ALLEGRO_LIBRARIES
        ${ALLEGRO_LIBRARY}
        ${ALLEGRO_MAIN_LIBRARY}
        ${ALLEGRO_IMAGE_LIBRARY}
        ${ALLEGRO_FONT_LIBRARY}
        ${ALLEGRO_TTF_LIBRARY}
        ${ALLEGRO_PRIMITIVES_LIBRARY}
        ${ALLEGRO_MEMFILE_LIBRARY}
        )
else(ALLEGRO_FOUND)
    set(ALLEGRO_LIBRARIES)
endif(ALLEGRO_FOUND)

mark_as_advanced(
    ALLEGRO_INCLUDE_DIR
    ALLEGRO_LIBRARY
    ALLEGRO_MAIN_LIBRARY
    ALLEGRO_IMAGE_LIBRARY
    ALLEGRO_FONT_LIBRARY
    ALLEGRO_TTF_LIBRARY
    ALLEGRO_PRIMITIVES_LIBRARY
    ALLEGRO_MEMFILE_LIBRARY
    )