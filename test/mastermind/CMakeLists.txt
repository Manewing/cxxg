set(TARGET test_mastermind)
add_executable(${TARGET}
  main.cpp
  ../../games/mastermind/src/MasterMind.cpp
)
add_test(unittest_mastermind ${TARGET})
target_link_libraries(${TARGET}
  cxxg
)
target_include_directories(${TARGET}
  PUBLIC ../../games/mastermind/include/
  PUBLIC ../common/
)
