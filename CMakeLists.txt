cmake_minimum_required(VERSION 3.20)
project(practise_cplusplus_net)

set(CMAKE_CXX_STANDARD 14)

include_directories(depends)
include_directories(depends/include)
include_directories(depends/json)
include_directories(depends/sqlite)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/lib)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -Wall -Wno-c4819")
endif ()

add_subdirectory(tcp_client)
add_subdirectory(tcp_server)
