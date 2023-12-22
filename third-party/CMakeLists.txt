message("Adding entt")
add_subdirectory(entt)

if (BUILD_TESTS)
message("Adding googletest")
add_subdirectory(googletest)
endif()

message("Adding ymir")
set(BUILD_TESTS OFF)
add_subdirectory(ymir)
