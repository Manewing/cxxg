set(TARGET common)

set(HEADER_FILES
  include/cxxg/Row.h
  include/cxxg/Screen.h
)

set(SOURCE_FILES
  src/Row.cpp
  src/Screen.cpp
)

add_library(${TARGET} STATIC ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${TARGET} PUBLIC include)
