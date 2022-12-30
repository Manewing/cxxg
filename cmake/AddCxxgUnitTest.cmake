function(add_cxxg_unittest)

set(options "")
set(oneValueArgs NAME RENAME)
set(multiValueArgs SOURCES INCLUDES LIBRARIES)

cmake_parse_arguments(
  ARGS
  "${options}"
  "${oneValueArgs}"
  "${multiValueArgs}"
  ${ARGN}
)

set(TARGET test_${ARGS_NAME})

add_executable(${TARGET}
  ${ARGS_SOURCES}
)

add_test(unittest_${ARGS_NAME} ${TARGET})

target_link_libraries(${TARGET}
  cxxg
  ${GTEST_LIBRARIES}
  ${GTEST_MAIN_LIBRARY}
  ${GMOCK_LIBRARIES}
  ${ARGS_LIBRARIES}
  pthread
)

target_include_directories(${TARGET}
  ${ARGS_INCLUDES}
  PUBLIC ${CMAKE_SOURCE_DIR}/test/common/
)

endfunction()
