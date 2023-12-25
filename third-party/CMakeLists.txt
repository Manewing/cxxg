message(STATUS "cxxg: Adding entt")
add_subdirectory(entt EXCLUDE_FROM_ALL)

if (BUILD_TESTS)
message(STATUS "cxxg: Adding googletest")
add_subdirectory(googletest EXCLUDE_FROM_ALL)
endif()

message(STATUS "cxxg: Adding ymir")
set(BUILD_TESTS OFF)
add_subdirectory(ymir EXCLUDE_FROM_ALL)
