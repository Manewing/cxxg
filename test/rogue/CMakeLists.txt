set(SOURCES
  CreatureDatabaseTest.cpp
  EventHubTest.cpp
  Components/HelpersTest.cpp
  Systems/StatsSystemTest.cpp
)

add_cxxg_unittest(
  NAME rogue
  SOURCES ${SOURCES}
  LIBRARIES librogue
)
