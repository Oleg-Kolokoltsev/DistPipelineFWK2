file(GLOB PID_THERMOSTAT_SRC
        "*.hpp"
        "*.cpp" )

add_executable (demo_thermostat "${PID_THERMOSTAT_SRC}")
target_link_libraries(demo_thermostat ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(demo_thermostat ${Boost_LIBRARIES})
target_link_libraries(demo_thermostat ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries(demo_thermostat ${Boost_SYSTEM_LIBRARY})
target_link_libraries(demo_thermostat ${Boost_THREAD_LIBRARY})
target_link_libraries(demo_thermostat ${GSL_LIBRARIES})
target_link_libraries(demo_thermostat ${ALLEGRO_LIBRARIES})
target_link_libraries(demo_thermostat lib_dsp lib_graphics)
set_property(TARGET demo_thermostat PROPERTY CXX_STANDARD 11)