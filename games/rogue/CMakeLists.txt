set(TARGET rogue)

set(YMIR_INSTALL "" CACHE STRING "ymir install location")
message("YMIR_INSTALL: ${YMIR_INSTALL}")

if(EXISTS "${YMIR_INSTALL}/include/ymir")


set(HEADER_FILES
  include/rogue/Components/AI.h
  include/rogue/Components/Entity.h
  include/rogue/Components/Level.h
  include/rogue/Components/Player.h
  include/rogue/Components/Stats.h
  include/rogue/Components/Transform.h
  include/rogue/Components/Visual.h
  include/rogue/Context.h
  include/rogue/Entity.h
  include/rogue/EventHub.h
  include/rogue/Game.h
  include/rogue/History.h
  include/rogue/Inventory.h
  include/rogue/Item.h
  include/rogue/ItemType.h
  include/rogue/ItemDatabase.h
  include/rogue/JSON.h
  include/rogue/Level.h
  include/rogue/LevelGenerator.h
  include/rogue/NPCEntity.h
  include/rogue/Parser.h
  include/rogue/Renderer.h
  include/rogue/Systems/AttackAISystem.h
  include/rogue/Systems/DeathSystem.h
  include/rogue/Systems/PlayerSystem.h
  include/rogue/Systems/RegenSystem.h
  include/rogue/Systems/StatsSystem.h
  include/rogue/Systems/System.h
  include/rogue/Systems/WanderAISystem.h
  include/rogue/UI/Controller.h
  include/rogue/UI/Controls.h
  include/rogue/UI/Decorator.h
  include/rogue/UI/Equipment.h
  include/rogue/UI/Frame.h
  include/rogue/UI/History.h
  include/rogue/UI/Inventory.h
  include/rogue/UI/ItemSelect.h
  include/rogue/UI/ListSelect.h
  include/rogue/UI/SelectBox.h
  include/rogue/UI/Stats.h
  include/rogue/UI/TextBox.h
  include/rogue/UI/Tooltip.h
  include/rogue/UI/Widget.h
  include/rogue/UI/WindowContainer.h
  include/rogue/UI/WordWrap.h
)

set(SOURCE_FILES
  src/Components/Buffs.cpp
  src/Components/Entity.cpp
  src/Components/Player.cpp
  src/Components/Stats.cpp
  src/Entity.cpp
  src/Game.cpp
  src/GameConfig.cpp
  src/History.cpp
  src/Inventory.cpp
  src/Item.cpp
  src/ItemType.cpp
  src/ItemDatabase.cpp
  src/JSON.cpp
  src/Level.cpp
  src/LevelGenerator.cpp
  src/NPCEntity.cpp
  src/Parser.cpp
  src/Renderer.cpp
  src/Systems/AgilitySystem.cpp
  src/Systems/AttackAISystem.cpp
  src/Systems/DeathSystem.cpp
  src/Systems/PlayerSystem.cpp
  src/Systems/RegenSystem.cpp
  src/Systems/StatsSystem.cpp
  src/Systems/WanderAISystem.cpp
  src/UI/Controller.cpp
  src/UI/Controls.cpp
  src/UI/Decorator.cpp
  src/UI/Equipment.cpp
  src/UI/Frame.cpp
  src/UI/History.cpp
  src/UI/Stats.cpp
  src/UI/Inventory.cpp
  src/UI/ItemSelect.cpp
  src/UI/ListSelect.cpp
  src/UI/SelectBox.cpp
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