set(TARGET npc_test)

set(SOURCE_FILES
  npc_test.cpp
)

add_executable(${TARGET}
  ${SOURCE_FILES}
)

target_link_libraries(${TARGET}
  librogue
)

set(TARGET proc_gen)

set(SOURCE_FILES
  proc_gen.cpp
)

add_executable(${TARGET}
  ${SOURCE_FILES}
)

target_link_libraries(${TARGET}
  librogue
)