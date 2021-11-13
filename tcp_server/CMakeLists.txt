cmake_minimum_required(VERSION 3.20)
project(tcp_server)

set(CMAKE_CXX_STANDARD 14)

#链接windows 静态库
link_libraries(ws2_32 wsock32)

add_executable(tcp_server main.cpp)
