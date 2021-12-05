//
// Created by chaoqi on 2021/11/21.
//

#ifndef PRACTISE_CPLUSPLUS_NET_CELL_NET_UTILS_HPP
#define PRACTISE_CPLUSPLUS_NET_CELL_NET_UTILS_HPP

#ifdef _WIN32
    #define FD_SETSIZE 10240
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
    #include <unistd.h>
#endif

#include <functional>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
#include <thread>
#include <memory>
#include <map>
#include <mutex>

#include "message_header.hpp"

using namespace std::chrono_literals;

const uint32_t RECV_BUF_SIZE = 4096 * 20;
const int CELL_THREAD_COUNT = 4;

void close_socket(SOCKET sock)
{
    if (sock != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(sock);
#else
        ::close(sock);
#endif
        sock = INVALID_SOCKET;
    }
}

class NetEnv
{
public:
    NetEnv()
    {
#ifdef _WIN32
        //prepare the Windows environment.
        WORD ver = MAKEWORD(2, 2);
        WSADATA data;
        WSAStartup(ver, &data);
#endif
    }

    ~NetEnv()
    {
#ifdef _WIN32
        //clean up the Windows environment.
        WSACleanup();
#endif
    }
};


#endif //PRACTISE_CPLUSPLUS_NET_CELL_NET_UTILS_HPP
