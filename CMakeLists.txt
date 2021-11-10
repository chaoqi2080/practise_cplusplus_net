cmake_minimum_required(VERSION 3.20)

project(longqi_tinynet)

set(CMAKE_CXX_STANDARD 11)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/lib)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -Wall -Wno-c4819")
endif ()

include_directories(depends)

add_subdirectory(server)
add_subdirectory(client)