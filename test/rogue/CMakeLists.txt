set(SOURCES
  EventHubTest.cpp
  ComponentHelpersTest.cpp
)

add_cxxg_unittest(
  NAME rogue
  SOURCES ${SOURCES}
  LIBRARIES librogue
)
