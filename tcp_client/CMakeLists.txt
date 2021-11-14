cmake_minimum_required(VERSION 3.20)
project(tcp_server)

set(CMAKE_CXX_STANDARD 14)

IF (WIN32)
    #链接windows 静态库
    link_libraries(ws2_32 wsock32)
ELSEIF (APPLE)
    MESSAGE(STATUS "Now is Apple systens.")
ELSEIF (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS's.")
    find_package( Threads )
ENDIF ()

add_executable(tcp_client main.cpp)

IF (UNIX)
    target_link_libraries( tcp_client ${CMAKE_THREAD_LIBS_INIT} )
ENDIF()