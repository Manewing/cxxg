set(SOURCES
  Components/BuffsTest.cpp
  Components/HelpersTest.cpp
  CraftingSystemTest.cpp
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
set(BUILD_DATA_DIR ${CMAKE_BINARY_DIR}/games/rogue/data)
set(SCHEMAS_DIR ${DATA_DIR}/schemas)
set(BUILD_SCHEMAS_DIR ${BUILD_DATA_DIR}/schemas)
set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/games/rogue/scripts)
set(SCHEMA_VALIDATOR ${SCRIPTS_DIR}/validator.py)

function(add_schema_test)
  set(options "")
  set(oneValueArgs NAME SCHEMA RENAME)
  set(multiValueArgs JSON_FILES)

  cmake_parse_arguments(
    ARGS
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
  )

  add_cxxg_test(
    NAME rogue_json_schema_${ARGS_NAME}
    COMMAND
      ${SCHEMA_VALIDATOR}
        --json ${ARGS_JSON_FILES}
        --schema ${ARGS_SCHEMA}
        --url "https://rogue-todo.com/"
        ${ARGS_RENAME}
  )
endfunction()


add_schema_test(
  NAME entity_db
  JSON_FILES ${BUILD_DATA_DIR}/entity_db.json
  SCHEMA ${SCHEMAS_DIR}/entity_db_schema.json
)

add_schema_test(
  NAME build_entity_db
  JSON_FILES ${BUILD_DATA_DIR}/entity_db.json
  SCHEMA ${BUILD_SCHEMAS_DIR}/entity_db_schema.json
)

add_schema_test(
  NAME item_db
  JSON_FILES ${BUILD_DATA_DIR}/item_db.json
  SCHEMA ${SCHEMAS_DIR}/item_db_schema.json
)

add_schema_test(
  NAME build_item_db
  JSON_FILES ${BUILD_DATA_DIR}/item_db.json
  SCHEMA ${BUILD_SCHEMAS_DIR}/item_db_schema.json
)

add_schema_test(
  NAME creature_db
  JSON_FILES ${BUILD_DATA_DIR}/creature_db.json
  SCHEMA ${SCHEMAS_DIR}/creature_db_schema.json
)

add_schema_test(
  NAME build_creature_db
  JSON_FILES ${BUILD_DATA_DIR}/creature_db.json
  SCHEMA ${BUILD_SCHEMAS_DIR}/creature_db_schema.json
)

add_schema_test(
  NAME level_db
  JSON_FILES ${BUILD_DATA_DIR}/level_db.json
  SCHEMA ${SCHEMAS_DIR}/level_db_schema.json
)

add_schema_test(
  NAME build_level_db
  JSON_FILES ${BUILD_DATA_DIR}/level_db.json
  SCHEMA ${BUILD_SCHEMAS_DIR}/level_db_schema.json
)

add_schema_test(
  NAME game_config
  JSON_FILES ${BUILD_DATA_DIR}/game_config.json
             ${BUILD_DATA_DIR}/test_game_config.json
  SCHEMA ${SCHEMAS_DIR}/game_config_schema.json
)

add_schema_test(
  NAME build_game_config
  JSON_FILES ${BUILD_DATA_DIR}/game_config.json
             ${BUILD_DATA_DIR}/test_game_config.json
  SCHEMA ${BUILD_SCHEMAS_DIR}/game_config_schema.json
)

file(GLOB LEVEL_FILES ${DATA_DIR}/levels/*.json)

add_schema_test(
  NAME level_configs
  JSON_FILES ${LEVEL_FILES}
  SCHEMA ${SCHEMAS_DIR}/level_config_schema.json
)

add_schema_test(
  NAME build_level_configs
  JSON_FILES ${LEVEL_FILES}
  SCHEMA ${BUILD_SCHEMAS_DIR}/level_config_schema.json
)