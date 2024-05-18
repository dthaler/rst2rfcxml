if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_STANDARD 20)
endif()

include_directories(../external/fmt/include)
include_directories(../external)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_library(lib STATIC "rst2rfcxml.h" "rst2rfcxml.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lib PROPERTY CXX_STANDARD 20)
endif()
