cmake_minimum_required(VERSION 3.5)
project(cxxg)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -ggdb"
)

add_subdirectory(games)
add_subdirectory(lib)

option(BUILD_TESTS "If to build unit tests (need google-test)" OFF)
if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
