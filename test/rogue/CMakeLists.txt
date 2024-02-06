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
  ItemsCommon.cpp
  ItemDatabaseTest.cpp
  ItemPrototypeTest.cpp
  ItemTest.cpp
  LevelDatabaseTest.cpp
  LevelGeneratorTest.cpp
  LootTableTest.cpp
  Systems/DeathSystemTest.cpp
  Systems/LOSSystemTest.cpp
  Systems/StatsSystemTest.cpp
  UI/WordWrapTest.cpp
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
      ${PYTHON_EXECUTABLE} ${SCHEMA_VALIDATOR}
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
  NAME crafting_db
  JSON_FILES ${BUILD_DATA_DIR}/crafting_db.json
  SCHEMA ${SCHEMAS_DIR}/crafting_db_schema.json
)

add_schema_test(
  NAME build_crafting_db
  JSON_FILES ${BUILD_DATA_DIR}/crafting_db.json
  SCHEMA ${BUILD_SCHEMAS_DIR}/crafting_db_schema.json
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

file(GLOB_RECURSE LEVEL_FILES ${DATA_DIR}/levels/*.json)

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

# Dump all items to stdout
add_cxxg_test(
  NAME rogue_loot_info_dump_items
  COMMAND ${CMAKE_BINARY_DIR}/games/rogue/loot_info
    ${BUILD_DATA_DIR}/item_db.json ${BUILD_SCHEMAS_DIR}/item_db_schema.json
)

# Dump specifc item
add_cxxg_test(
  NAME rogue_loot_info_dump_item
  COMMAND ${CMAKE_BINARY_DIR}/games/rogue/loot_info
    ${BUILD_DATA_DIR}/item_db.json ${BUILD_SCHEMAS_DIR}/item_db_schema.json
    --dump-item Blueberry
)

# Dump all loot tables
add_cxxg_test(
  NAME rogue_loot_info_dump_tables
  COMMAND ${CMAKE_BINARY_DIR}/games/rogue/loot_info
    ${BUILD_DATA_DIR}/item_db.json ${BUILD_SCHEMAS_DIR}/item_db_schema.json
    --dump-tables
)

# Dump specific loot table
add_cxxg_test(
  NAME rogue_loot_info_loot_table
  COMMAND ${CMAKE_BINARY_DIR}/games/rogue/loot_info
    ${BUILD_DATA_DIR}/item_db.json ${BUILD_SCHEMAS_DIR}/item_db_schema.json
    --loot-table common_chest 1000
)

# Run sanity check creating all items multiple times
add_cxxg_test(
  NAME rogue_loot_info_create_all_items
  COMMAND
    ${CMAKE_BINARY_DIR}/games/rogue/loot_info
    ${BUILD_DATA_DIR}/item_db.json ${BUILD_SCHEMAS_DIR}/item_db_schema.json
    --create-all-items 1000
)