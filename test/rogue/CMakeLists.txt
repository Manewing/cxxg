set(SOURCES
  Components/HelpersTest.cpp
  CreatureDatabaseTest.cpp
  EventHubTest.cpp
  LootTableTest.cpp
  Systems/StatsSystemTest.cpp
)

add_cxxg_unittest(
  NAME rogue
  SOURCES ${SOURCES}
  LIBRARIES librogue
)
