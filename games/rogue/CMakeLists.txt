set(TARGET rogue)

add_subdirectory(entt)

set(YMIR_INSTALL "" CACHE STRING "ymir install location")
message("YMIR_INSTALL: ${YMIR_INSTALL}")

if(EXISTS "${YMIR_INSTALL}/include/ymir")


set(HEADER_FILES
  include/Components/AI.h
  include/Components/Entity.h
  include/Components/Level.h
  include/Components/Player.h
  include/Components/Stats.h
  include/Components/Transform.h
  include/Components/Visual.h
  include/Context.h
  include/Entity.h
  include/EventHub.h
  include/Game.h
  include/History.h
  include/Inventory.h
  include/Item.h
  include/ItemDatabase.h
  include/Level.h
  include/LevelGenerator.h
  include/NPCEntity.h
  include/Parser.h
  include/Renderer.h
  include/Systems/AttackAISystem.h
  include/Systems/DeathSystem.h
  include/Systems/PlayerSystem.h
  include/Systems/RegenSystem.h
  include/Systems/System.h
  include/Systems/WanderAISystem.h
  include/UIController.h
  include/UIListSelect.h
)

set(SOURCE_FILES
  src/Components/Entity.cpp
  src/Components/Player.cpp
  src/Components/Stats.cpp
  src/Entity.cpp
  src/Game.cpp
  src/History.cpp
  src/Inventory.cpp
  src/Item.cpp
  src/ItemDatabase.cpp
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
  src/Systems/WanderAISystem.cpp
  src/UIController.cpp
  src/UIListSelect.cpp
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
target_include_directories(lib${TARGET} PUBLIC ${YMIR_INSTALL}/include)
##

target_link_libraries(lib${TARGET}
  cxxg ${LIBYMIR} EnTT::EnTT
)

target_link_libraries(${TARGET}
  lib${TARGET}
)

add_subdirectory(test)

else()

message("Skipping target '${TARGET}' missing libymir")

endif()