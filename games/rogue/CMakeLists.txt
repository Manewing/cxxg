set(TARGET rogue)

option(YMIR_INSTALL "ymir install location" "")
message("YMIR_INSTALL: ${YMIR_INSTALL}")

if(EXISTS "${YMIR_INSTALL}/include/ymir")


set(HEADER_FILES
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
  include/UIController.h
  include/UIListSelect.h
)

set(SOURCE_FILES
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
  cxxg ${LIBYMIR}
)

target_link_libraries(${TARGET}
  lib${TARGET}
)

add_subdirectory(test)

else()

message("Skipping target '${TARGET}' missing libymir")

endif()