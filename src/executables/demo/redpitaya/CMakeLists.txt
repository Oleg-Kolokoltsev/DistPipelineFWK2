
file(GLOB REDPITAYA_SRC
        "*.hpp"
        "*.cpp" )

add_executable (demo_redpitaya ${REDPITAYA_SRC})
target_link_libraries(demo_redpitaya ${Boost_LIBRARIES})
target_link_libraries(demo_redpitaya ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries(demo_redpitaya ${Boost_SYSTEM_LIBRARY})
target_link_libraries(demo_redpitaya ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(demo_redpitaya ${ALLEGRO_LIBRARIES})
target_link_libraries(demo_redpitaya ${GSL_LIBRARIES})
target_link_libraries(demo_redpitaya lib_dsp lib_graphics)
set_property(TARGET demo_redpitaya PROPERTY CXX_STANDARD 11)