set(TARGET rogue)

option(YMIR_INSTALL "ymir install location" "")
message("YMIR_INSTALL: ${YMIR_INSTALL}")

if(EXISTS "${YMIR_INSTALL}/include/ymir")


set(HEADER_FILES
  include/Game.h
  include/Level.h
  include/LevelGenerator.h
  include/Parser.h
)

set(SOURCE_FILES
  src/Game.cpp
  src/Level.cpp
  src/LevelGenerator.cpp
  src/Parser.cpp
  src/main.cpp
)

add_executable(${TARGET}
  ${HEADER_FILES}
  ${SOURCE_FILES}
)


target_include_directories(${TARGET} PUBLIC include)


## YMIR
find_library(LIBYMIR NAMES ymir HINTS "${YMIR_INSTALL}/lib")
target_include_directories(${TARGET} PUBLIC ${YMIR_INSTALL}/include)
##

target_link_libraries(${TARGET}
  cxxg ${LIBYMIR}
)

else()

message("Skipping target '${TARGET}' missing libymir")

endif()