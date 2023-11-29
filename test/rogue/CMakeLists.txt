set(SOURCES
  Components/BuffsTest.cpp
  Components/HelpersTest.cpp
  CreatureDatabaseTest.cpp
  EntityDatabaseHelpersTest.cpp
  EntityDatabaseTest.cpp
  EntityFactoryTest.cpp
  EquipmentTest.cpp
  EventHubTest.cpp
  GameWorldTest.cpp
  InventoryHandlerTest.cpp
  InventoryTest.cpp
  ItemDatabaseTest.cpp
  LevelDatabaseTest.cpp
  LevelGeneratorTest.cpp
  LootTableTest.cpp
  Systems/DeathSystemTest.cpp
  Systems/LOSSystemTest.cpp
  Systems/StatsSystemTest.cpp
)

add_cxxg_unittest(
  NAME rogue
  SOURCES ${SOURCES}
  LIBRARIES librogue
)

set(DATA_DIR ${CMAKE_SOURCE_DIR}/games/rogue/data)
set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/games/rogue/scripts)
set(SCHEMA_VALIDATOR ${SCRIPTS_DIR}/validator.py)

add_cxxg_test(
  NAME rogue_json_schema_game_cfg
  COMMAND
    ${SCHEMA_VALIDATOR}
      --json ${DATA_DIR}/game_config.json ${DATA_DIR}/test_game_config.json
      --schema ${DATA_DIR}/game_config_schema.json
)

add_cxxg_test(
  NAME rogue_json_schema_item_db
  COMMAND
    ${SCHEMA_VALIDATOR}
      --json ${DATA_DIR}/item_db.json
      --schema ${DATA_DIR}/item_db_schema.json
)

add_cxxg_test(
  NAME rogue_json_schema_creature_db
  COMMAND
    ${SCHEMA_VALIDATOR}
      --json ${DATA_DIR}/creature_db.json
      --schema ${DATA_DIR}/creature_db_schema.json
)

add_cxxg_test(
  NAME rogue_json_schema_level_db
  COMMAND
    ${SCHEMA_VALIDATOR}
      --json ${DATA_DIR}/level_db.json
      --schema ${DATA_DIR}/level_db_schema.json
)

file(GLOB LEVEL_FILES ${DATA_DIR}/levels/*.json)
add_cxxg_test(
  NAME rogue_json_schema_levels
  COMMAND
    ${SCHEMA_VALIDATOR}
      --json ${LEVEL_FILES}
      --schema ${DATA_DIR}/level_config_schema.json
)
