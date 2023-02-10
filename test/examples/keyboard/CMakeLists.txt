set(TARGET keyboard)

set(HEADER_FILES
)

set(SOURCE_FILES
  keyboard.cpp
)

add_executable(${TARGET}
  ${HEADER_FILES}
  ${SOURCE_FILES}
)

target_link_libraries(${TARGET}
  cxxg
)