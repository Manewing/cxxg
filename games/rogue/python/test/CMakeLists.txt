set(PYTHONPATH ${CMAKE_SOURCE_DIR}/games/rogue/python)

add_test(NAME pyrogue_tests
  COMMAND ${CMAKE_COMMAND} -E env
    PYTHONPATH=${PYTHONPATH}
    ${PYTHON_EXECUTABLE} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR}/pyrogue)