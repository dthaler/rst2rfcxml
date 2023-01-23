# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("rst2rfcxml")

# Include sub-projects.
add_subdirectory("external/fmt")
add_subdirectory("rst2rfcxml")
