set(TARGET cxxg)

set(HEADER_FILES
  include/cxxg/Game.h
  include/cxxg/Row.h
  include/cxxg/Screen.h
  include/cxxg/Types.h
  include/cxxg/Utils.h
)

set(SOURCE_FILES
  src/Game.cpp
  src/Row.cpp
  src/Screen.cpp
  src/Types.cpp
  src/Utils.cpp
)

if (WIN32)
  list(APPEND SOURCE_FILES "src/UtilsWin.cpp")
else()
  list(APPEND SOURCE_FILES "src/UtilsUnix.cpp")
endif()

add_library(${TARGET} STATIC ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${TARGET} PUBLIC include)
