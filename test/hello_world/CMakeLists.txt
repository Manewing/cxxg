set(TARGET test_hello_world)
add_executable(${TARGET}
  main.cpp
)
add_test(unittest_hello_world ${TARGET})
target_link_libraries(${TARGET}
  cxxg
)
