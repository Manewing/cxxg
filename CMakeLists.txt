cmake_minimum_required(VERSION 3.5)
project(cxxg)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMMON_COMPILE_OPTIONS
  -Wall
  $<$<CONFIG:RELEASE>:-Werror>
)

enable_testing()

add_subdirectory(common)
add_subdirectory(test)
