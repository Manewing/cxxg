set(TARGET 2048)

set(HEADER_FILES
  include/Game2048.h
)

set(SOURCE_FILES
  src/Game2048.cpp
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
