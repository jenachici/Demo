SET(TARGET public)

include_directories("${CMAKE_SOURCE_DIR}")

aux_source_directory(. SRC_LIST)
add_library(${TARGET} STATIC ${SRC_LIST})
