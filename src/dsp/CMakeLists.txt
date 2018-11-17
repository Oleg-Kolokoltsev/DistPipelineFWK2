project(LibDSP)

file(GLOB LIB_DSP_SRC
        "*.hpp"
        "*.cpp"
        "fractional_fourier/*.cpp"
        "fourier/*.cpp"
        "interpolation/*.cpp")

add_library(lib_dsp STATIC ${LIB_DSP_SRC})

target_include_directories(lib_dsp PUBLIC "fourier")
target_include_directories(lib_dsp PUBLIC "fractional_fourier")
target_include_directories(lib_dsp PUBLIC "interpolation")

target_link_libraries(lib_dsp)
target_link_libraries(lib_dsp ${GSL_LIBRARIES})

if(CUDA_FOUND)
    target_link_libraries(lib_dsp ${CUDA_LIBRARIES})
    if(HAVE_CUBLAS_V2_H)
        CUDA_ADD_CUBLAS_TO_TARGET(lib_dsp)
    endif(HAVE_CUBLAS_V2_H)
endif(CUDA_FOUND)

set_property(TARGET lib_dsp PROPERTY CXX_STANDARD 11)