set(SOURCES
  EventHubTest.cpp
)

add_cxxg_unittest(
  NAME rogue
  SOURCES ${SOURCES}
  LIBRARIES librogue
)
