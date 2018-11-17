project(LibGraphics)

file(GLOB LIB_GRAPHICS_SRC
        "*.hpp"
        "*.cpp"
        "scope_messages/*.cpp"
        "scope_messages/*.hpp")

#STATIC ???
add_library(lib_graphics STATIC ${LIB_GRAPHICS_SRC})
set_target_properties(lib_graphics PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(lib_graphics PUBLIC "scope_messages")
target_link_libraries(lib_graphics ${ALLEGRO_LIBRARIES})
target_link_libraries(lib_graphics ${Boost_LIBRARIES})
target_link_libraries(lib_graphics ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries(lib_graphics ${Boost_SYSTEM_LIBRARY})
target_link_libraries(lib_graphics ${Boost_THREAD_LIBRARY})
target_link_libraries(lib_graphics lib_dsp)
set_property(TARGET lib_graphics PROPERTY CXX_STANDARD 11)