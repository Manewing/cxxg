set(TARGET test_colors)
add_executable(${TARGET}
  main.cpp
)
add_test(unittest_colors ${TARGET})
target_link_libraries(${TARGET}
  cxxg
)
