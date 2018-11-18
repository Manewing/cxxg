set(SOURCES
  main.cpp
  ../../games/mastermind/src/MasterMind.cpp
)

set(INCLUDES
  PUBLIC ../../games/mastermind/include/
)

add_cxxg_unittest(
  NAME mastermind
  SOURCES ${SOURCES}
  INCLUDES ${INCLUDES}
)
