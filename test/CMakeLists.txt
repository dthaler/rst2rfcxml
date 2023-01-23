if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_STANDARD 20)
endif()

include_directories(../external/Catch2/single_include/catch2)
include_directories(../external/fmt/include)
include_directories(../external)
include_directories(../lib)

add_executable(tests "test.cpp" "../lib/rst2rfcxml.h" "basic_tests.cpp")
target_link_libraries(tests PRIVATE fmt::fmt-header-only)
target_link_libraries(tests PRIVATE lib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET tests PROPERTY CXX_STANDARD 20)
endif()
