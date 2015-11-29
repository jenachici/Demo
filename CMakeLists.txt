project(Server)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -msse -msse2 -msse3 -msse4.1   -g3  -fPIC -Wall -Wextra -Wno-unused-variable -Wno-sign-compare -Wno-unused-function -Wno-unused-local-typedefs -Wno-unused-parameter -rdynamic -fpermissive -std=c++11 ")


set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_ALL "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")

if(CMAKE_BUILD_RELEASE)
    message("set release model")
    add_definitions(-DRELEASE)
    set(CMAKE_BUILD_TYPE relwithdebinfo)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_ALL "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

include(thirdparty/CMakeLists.txt)
add_subdirectory(server)
add_subdirectory(public)

