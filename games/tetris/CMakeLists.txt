set(TARGET tetris)

set(HEADER_FILES
  include/Tetris.h
  include/Tetromino.h
)

set(SOURCE_FILES
  src/Tetris.cpp
  src/Tetromino.cpp
  src/Field.cpp
  src/main.cpp
)

add_executable(${TARGET}
  ${HEADER_FILES}
  ${SOURCE_FILES}
)

target_link_libraries(${TARGET}
  cxxg
)

target_include_directories(${TARGET} PUBLIC include)

install(TARGETS ${TARGET} DESTINATION bin)