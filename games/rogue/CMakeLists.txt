set(TARGET rogue)

set(HEADER_FILES
  include/rogue/Components/AI.h
  include/rogue/Components/Combat.h
  include/rogue/Components/Entity.h
  include/rogue/Components/LOS.h
  include/rogue/Components/Level.h
  include/rogue/Components/Player.h
  include/rogue/Components/RaceFaction.h
  include/rogue/Components/Stats.h
  include/rogue/Components/Transform.h
  include/rogue/Components/Visual.h
  include/rogue/Context.h
  include/rogue/CraftingDatabase.h
  include/rogue/CraftingHandler.h
  include/rogue/EntityAssemblers.h
  include/rogue/EntityDatabase.h
  include/rogue/Equipment.h
  include/rogue/Event.h
  include/rogue/EventHub.h
  include/rogue/Game.h
  include/rogue/GameWorld.h
  include/rogue/History.h
  include/rogue/Inventory.h
  include/rogue/InventoryHandler.h
  include/rogue/Item.h
  include/rogue/ItemDatabase.h
  include/rogue/ItemEffect.h
  include/rogue/ItemEffectImpl.h
  include/rogue/ItemPrototype.h
  include/rogue/ItemSpecialization.h
  include/rogue/ItemType.h
  include/rogue/JSON.h
  include/rogue/JSONHelpers.h
  include/rogue/Level.h
  include/rogue/LevelDatabase.h
  include/rogue/LevelGenerator.h
  include/rogue/LootTable.h
  include/rogue/Parser.h
  include/rogue/Renderer.h
  include/rogue/Systems/AttackAISystem.h
  include/rogue/Systems/CombatSystem.h
  include/rogue/Systems/DeathSystem.h
  include/rogue/Systems/LOSSystem.h
  include/rogue/Systems/MovementSystem.h
  include/rogue/Systems/NPCSystem.h
  include/rogue/Systems/PlayerSystem.h
  include/rogue/Systems/RegenSystem.h
  include/rogue/Systems/StatsSystem.h
  include/rogue/Systems/System.h
  include/rogue/Systems/WanderAISystem.h
  include/rogue/UI/Buffs.h
  include/rogue/UI/CommandLine.h
  include/rogue/UI/Controller.h
  include/rogue/UI/Controls.h
  include/rogue/UI/Crafting.h
  include/rogue/UI/Decorator.h
  include/rogue/UI/Equipment.h
  include/rogue/UI/Frame.h
  include/rogue/UI/History.h
  include/rogue/UI/Interact.h
  include/rogue/UI/Inventory.h
  include/rogue/UI/Item.h
  include/rogue/UI/ItemSelect.h
  include/rogue/UI/ListSelect.h
  include/rogue/UI/Menu.h
  include/rogue/UI/SelectBox.h
  include/rogue/UI/Stats.h
  include/rogue/UI/TargetUI.h
  include/rogue/UI/TextBox.h
  include/rogue/UI/Tooltip.h
  include/rogue/UI/Widget.h
  include/rogue/UI/WindowContainer.h
  include/rogue/UI/WordWrap.h
)

set(SOURCE_FILES
  src/Components/AI.cpp
  src/Components/Buffs.cpp
  src/Components/Combat.cpp
  src/Components/Entity.cpp
  src/Components/Player.cpp
  src/Components/RaceFaction.cpp
  src/Components/Stats.cpp
  src/Components/Visual.cpp
  src/CraftingDatabase.cpp
  src/CraftingHandler.cpp
  src/EntityAssemblers.cpp
  src/EntityDatabase.cpp
  src/Equipment.cpp
  src/Event.cpp
  src/Game.cpp
  src/GameConfig.cpp
  src/GameWorld.cpp
  src/History.cpp
  src/Inventory.cpp
  src/InventoryHandler.cpp
  src/Item.cpp
  src/ItemDatabase.cpp
  src/ItemEffect.cpp
  src/ItemEffectImpl.cpp
  src/ItemPrototype.cpp
  src/ItemSpecialization.cpp
  src/ItemType.cpp
  src/JSON.cpp
  src/JSONHelpers.cpp
  src/Level.cpp
  src/LevelDatabase.cpp
  src/LevelGenerator.cpp
  src/LootTable.cpp
  src/Parser.cpp
  src/Renderer.cpp
  src/Systems/AgilitySystem.cpp
  src/Systems/AttackAISystem.cpp
  src/Systems/CombatSystem.cpp
  src/Systems/DeathSystem.cpp
  src/Systems/LOSSystem.cpp
  src/Systems/MovementSystem.cpp
  src/Systems/NPCSystem.cpp
  src/Systems/PlayerSystem.cpp
  src/Systems/RegenSystem.cpp
  src/Systems/StatsSystem.cpp
  src/Systems/WanderAISystem.cpp
  src/UI/Buffs.cpp
  src/UI/CommandLine.cpp
  src/UI/Controller.cpp
  src/UI/Controls.cpp
  src/UI/Crafting.cpp
  src/UI/Decorator.cpp
  src/UI/Equipment.cpp
  src/UI/Frame.cpp
  src/UI/History.cpp
  src/UI/Interact.cpp
  src/UI/Inventory.cpp
  src/UI/Item.cpp
  src/UI/ItemSelect.cpp
  src/UI/ListSelect.cpp
  src/UI/Menu.cpp
  src/UI/SelectBox.cpp
  src/UI/Stats.cpp
  src/UI/TargetUI.cpp
  src/UI/TextBox.cpp
  src/UI/Tooltip.cpp
  src/UI/Widget.cpp
  src/UI/WindowContainer.cpp
  src/UI/WordWrap.cpp
)

add_library(
  lib${TARGET} STATIC
  ${HEADER_FILES}
  ${SOURCE_FILES}
)

add_executable(${TARGET}
  src/main.cpp
)

target_include_directories(lib${TARGET} PUBLIC include)

target_link_libraries(lib${TARGET}
  cxxg ymir EnTT::EnTT
)

target_include_directories(lib${TARGET}
  PUBLIC
    ${YMIR_INSTALL}/include
)
target_include_directories(lib${TARGET} SYSTEM
  PUBLIC
    ../../third-party/rapidjson/include
)

target_link_libraries(${TARGET}
  lib${TARGET}
)

add_subdirectory(test)

add_custom_target(
  rogue_create_data_dir
  ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/data/
)

file(GLOB DATA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/*.json ${CMAKE_CURRENT_SOURCE_DIR}/data/*.yaml)
file(GLOB SCHEMA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/schemas/*.json)
file(GLOB LEVEL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/levels/*.json)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/data/

  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_BINARY_DIR}/data

  # Post-process all JSON schemas
  COMMAND
    ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/schema_processor.py
    --schema-files
        ${SCHEMA_FILES}
    --output-dir
      ${CMAKE_CURRENT_BINARY_DIR}/data/schemas

  # Post-process all YAML data files
  COMMAND
    ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/yaml_json.py
      ${CMAKE_CURRENT_SOURCE_DIR}/data/crafting_db.yaml
      ${CMAKE_CURRENT_BINARY_DIR}/data/crafting_db.json


  # Post-process JSON configurations
  COMMAND
   ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/entity_db_post_process.py
    --entity-db
      ${CMAKE_CURRENT_SOURCE_DIR}/data/entity_db.json
    --output
      ${CMAKE_CURRENT_BINARY_DIR}/data/entity_db.json

  # Make JSON schemas available for tests
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_BINARY_DIR}/data/schemas
    ${CMAKE_BINARY_DIR}/test/rogue/schemas/

  DEPENDS
      ${SCHEMA_FILES}
      ${DATA_FILES}
      ${LEVEL_FILES}
      ${CMAKE_CURRENT_SOURCE_DIR}/scripts/schema_processor.py
)

install(
  TARGETS ${TARGET}
  RUNTIME DESTINATION bin
)

install(
  TARGETS lib${TARGET}
  ARCHIVE DESTINATION lib
)

install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data/
  DESTINATION shared/data
)