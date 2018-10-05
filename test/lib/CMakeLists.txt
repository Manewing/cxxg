# unit test hello world
set(TARGET test_hello_world)
add_executable(${TARGET}
  hello_world.cpp
)
add_test(unittest_hello_world ${TARGET})
target_link_libraries(${TARGET}
  cxxg
)

# unit test for colors
set(TARGET test_colors)
add_executable(${TARGET}
  colors.cpp
)
add_test(unittest_colors ${TARGET})
target_link_libraries(${TARGET}
  cxxg
)


