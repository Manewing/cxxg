set(TARGET rogue)

set(YMIR_INSTALL "" CACHE STRING "ymir install location")
message("YMIR_INSTALL: ${YMIR_INSTALL}")

if(EXISTS "${YMIR_INSTALL}/include/ymir")


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
  include/rogue/CreatureDatabase.h
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
  include/rogue/LevelDatabase.h
  include/rogue/ItemPrototype.h
  include/rogue/ItemSpecialization.h
  include/rogue/ItemType.h
  include/rogue/JSON.h
  include/rogue/JSONHelpers.h
  include/rogue/Level.h
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
  include/rogue/UI/Decorator.h
  include/rogue/UI/Equipment.h
  include/rogue/UI/Frame.h
  include/rogue/UI/History.h
  include/rogue/UI/Interact.h
  include/rogue/UI/Inventory.h
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
  src/CreatureDatabase.cpp
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
  src/ItemPrototype.cpp
  src/LevelDatabase.cpp
  src/ItemSpecialization.cpp
  src/ItemType.cpp
  src/JSON.cpp
  src/JSONHelpers.cpp
  src/Level.cpp
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
  src/UI/Decorator.cpp
  src/UI/Equipment.cpp
  src/UI/Frame.cpp
  src/UI/History.cpp
  src/UI/Interact.cpp
  src/UI/Inventory.cpp
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

## YMIR
find_library(LIBYMIR NAMES ymir HINTS "${YMIR_INSTALL}/lib")
##

target_link_libraries(lib${TARGET}
  cxxg ${LIBYMIR} EnTT::EnTT
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

else()

message("Skipping target '${TARGET}' missing libymir")

endif()


add_custom_command(TARGET ${TARGET} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_BINARY_DIR}/build/data

  COMMAND
    ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/schema_processor.py
    --schema-files
      ${CMAKE_CURRENT_SOURCE_DIR}/data/entity_db_schema.json
      ${CMAKE_CURRENT_SOURCE_DIR}/data/item_db_schema.json
      ${CMAKE_CURRENT_SOURCE_DIR}/data/level_db_schema.json
      ${CMAKE_CURRENT_SOURCE_DIR}/data/game_config_schema.json
      ${CMAKE_CURRENT_SOURCE_DIR}/data/level_config_schema.json
    --output-dir
      ${CMAKE_CURRENT_BINARY_DIR}/data

  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data/levels
    ${CMAKE_CURRENT_BINARY_DIR}/data/levels
)

