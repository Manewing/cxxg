cmake_minimum_required(VERSION 3.20)
project(cxxg)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(cmake/AddCxxgTest.cmake)
include(cmake/AddCxxgUnitTest.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  # Make sure runtime libraries are included in the install for Windows
  include(InstallRequiredSystemLibraries)
else()
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -ggdb"
  )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT PYTHON_EXECUTABLE)
  find_package(Python3 COMPONENTS Interpreter)
  set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
endif()
message(STATUS "Python executable: ${PYTHON_EXECUTABLE}")

add_subdirectory(games)
add_subdirectory(lib)
add_subdirectory(third-party)

option(BUILD_TESTS "If to build unit tests (need google-test)" OFF)
if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
