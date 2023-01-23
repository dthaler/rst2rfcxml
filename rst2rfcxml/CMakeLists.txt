# CMakeList.txt : CMake project for rst2rfcxml, include source and define
# project specific logic here.
#

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_STANDARD 20)
endif()

include_directories(../external/fmt/include)
include_directories(../external)
include_directories(../lib)

# Add source to this project's executable.
add_executable (rst2rfcxml "main.cpp" "../lib/rst2rfcxml.h")
target_link_libraries(rst2rfcxml PRIVATE fmt)
target_link_libraries(rst2rfcxml PRIVATE lib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET rst2rfcxml PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
