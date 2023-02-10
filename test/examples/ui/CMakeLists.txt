set(TARGET ui)

set(HEADER_FILES
)

set(SOURCE_FILES
  ui.cpp
)

add_executable(${TARGET}
  ${HEADER_FILES}
  ${SOURCE_FILES}
)

target_link_libraries(${TARGET}
  cxxg
  librogue # FIXME base UI components need to be moved
)