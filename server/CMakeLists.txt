SET(TARGET server)

include_directories("${CMAKE_SOURCE_DIR}")

aux_source_directory(. SRC_LIST)
add_executable(${TARGET} ${SRC_LIST})

target_link_libraries(${TARGET} glog
                                gflags
                                unwind)

target_link_libraries(${TARGET} protobuf
                                pthread rt m  dl
                                )

set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++ -Wl,--no-undefined")
