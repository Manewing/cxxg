function(add_cxxg_test)

set(options "")
set(oneValueArgs NAME)
set(multiValueArgs COMMAND)

cmake_parse_arguments(
  ARGS
  "${options}"
  "${oneValueArgs}"
  "${multiValueArgs}"
  ${ARGN}
)

add_test(NAME test_${ARGS_NAME}
         COMMAND ${ARGS_COMMAND})

endfunction()

