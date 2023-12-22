set(SOURCES
  main.cpp
  ../../games/2048/src/Game2048.cpp
)

set(INCLUDES
  PUBLIC ../../games/2048/include/
)

add_cxxg_unittest(
  NAME 2048
  SOURCES ${SOURCES}
  INCLUDES ${INCLUDES}
  LIBRARIES cxxg
)
