project(LibCore)

file(GLOB LIB_CORE_SRC
        "*.hpp"
        "*.cpp" )

#lib core is empty for the moment because there is no *.cpp files
#STATIC ???
#add_library(lib_core STATIC ${LIB_CORE_SRC})
#set_target_properties(lib_core PROPERTIES LINKER_LANGUAGE CXX)
#set_property(TARGET lib_core PROPERTY CXX_STANDARD 11)