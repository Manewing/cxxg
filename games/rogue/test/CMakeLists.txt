set(TARGET npc_test)

set(SOURCE_FILES
  main.cpp
)

add_executable(${TARGET}
  ${SOURCE_FILES}
)

target_link_libraries(${TARGET}
  librogue
)