set(TARGET mastermind)

set(HEADER_FILES
  include/MasterMind.h
)

set(SOURCE_FILES
  src/MasterMind.cpp
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
