set(TARGET test_2048)
add_executable(${TARGET}
  main.cpp
  ../../games/2048/src/Game2048.cpp
)
add_test(unittest_2048 ${TARGET})
target_link_libraries(${TARGET}
  cxxg
)
target_include_directories(${TARGET}
  PUBLIC ../../games/2048/include/
  PUBLIC ../common/
)
