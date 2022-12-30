find_package(GTest "1.8.0" REQUIRED)
find_package(GMock "1.8.0" REQUIRED)
include_directories(SYSTEM ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

add_subdirectory(2048)
add_subdirectory(lib)
add_subdirectory(mastermind)
add_subdirectory(rogue)
