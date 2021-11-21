//
// Created by chaoqi on 2021/11/21.
//

#ifndef PRACTISE_CPLUSPLUS_NET_CELL_NET_UTILS_HPP
#define PRACTISE_CPLUSPLUS_NET_CELL_NET_UTILS_HPP

#ifdef _WIN32
    #define FD_SETSIZE 1024
    #define WIN32_LEAN_AND_MEAN
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <windows.h>
    #include <WinSock2.h>
#else
#define INVALID_SOCKET  (SOCKET)(~0)
    #define SOCKET_ERROR            (-1)
    #define SOCKET int32_t
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <cstdio>
    #include <cstring>
    #include <unistd.h>
#endif

#include <thread>
#include <memory>

const uint32_t RECV_BUF_SIZE = 4096 * 20;


#endif //PRACTISE_CPLUSPLUS_NET_CELL_NET_UTILS_HPP
