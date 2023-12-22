# unit test hello world
add_cxxg_unittest(
  NAME cxxg_hello_world
  SOURCES hello_world.cpp
  INCLUDES
  LIBRARIES cxxg
)

# unit test for general tests
add_cxxg_unittest(
  NAME cxxg_general
  SOURCES accesses.cpp colors.cpp
  INCLUDES
  LIBRARIES cxxg
)
