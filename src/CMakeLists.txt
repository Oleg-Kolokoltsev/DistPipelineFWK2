
# order matters here, because for example LibDSP uses LibCore, etc.
add_subdirectory(core)
include_directories(${LibCore_SOURCE_DIR})

add_subdirectory(graphics)
include_directories(${LibGraphics_SOURCE_DIR})

add_subdirectory(dsp)
include_directories(${LibDSP_SOURCE_DIR})

add_subdirectory(sources)
include_directories(${LibSources_SOURCE_DIR})

add_subdirectory(executables)


add_executable (test_concept test_concept.cpp)
target_link_libraries(test_concept ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test_concept ${Boost_LIBRARIES})
target_link_libraries(test_concept ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries(test_concept ${Boost_SYSTEM_LIBRARY})
target_link_libraries(test_concept ${Boost_THREAD_LIBRARY})
target_link_libraries(test_concept ${ALLEGRO_LIBRARIES})
target_link_libraries(test_concept ${GSL_LIBRARIES})
target_link_libraries(test_concept lib_dsp lib_graphics)
set_property(TARGET test_concept PROPERTY CXX_STANDARD 11)

#add_executable(net_cdf net_cdf.cpp)
#target_link_libraries(net_cdf ${NETCDF_LIBRARIES_CXX})
#set_property(TARGET net_cdf PROPERTY CXX_STANDARD 11)