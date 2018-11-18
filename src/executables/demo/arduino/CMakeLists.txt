#add_subdirectory(board)
#add_subdirectory(host)

set(RELATIVE_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE "${CMAKE_SOURCE_DIR}" "${PROJECT_NAME}" RELATIVE_CURRENT_DIR ${RELATIVE_CURRENT_DIR})
message(STATUS "For additional demos see: ${RELATIVE_CURRENT_DIR}/CMakeLists.txt")
