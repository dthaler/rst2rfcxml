# CMakeList.txt : CMake project for rst2rfcxml, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (rst2rfcxml "rst2rfcxml.cpp" "rst2rfcxml.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET rst2rfcxml PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
