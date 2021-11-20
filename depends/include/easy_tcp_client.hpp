//
// Created by chaoqi on 2021/11/20.
//

#ifndef PRACTISE_CPLUSPLUS_NET_EASY_TCP_CLIENT_HPP
#define PRACTISE_CPLUSPLUS_NET_EASY_TCP_CLIENT_HPP

#ifdef _WIN32
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

#include "message_header.hpp"

class EasyTcpClient
{
public:
    EasyTcpClient()
    {

    }

    virtual ~EasyTcpClient()
    {
        close();
    }

    int connect(const char* ip, unsigned short port)
    {
        if (_sock == INVALID_SOCKET)
        {
            init_socket();
        }
        sockaddr_in sock_in = {};
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons(port);
#ifdef _WIN32
        sock_in.sin_addr.S_un.S_addr = inet_addr(ip);
#else
        sock_in.sin_addr.s_addr = inet_addr(ip);
#endif
        int ret = ::connect(_sock, (sockaddr*)&sock_in, sizeof(sock_in));
        if (SOCKET_ERROR == ret) {
            printf("connect server error\n");
        }
        return ret;
    }

    void close()
    {
        if (_sock == INVALID_SOCKET)
        {
            return;
        }
#ifdef _WIN32
        WSACleanup();
#endif
        close_socket(_sock);
    }

    bool on_run()
    {
        if (!is_run())
        {
            printf("on_run client is not run\n");
            return false;
        }

        fd_set read_fds;
        fd_set write_fds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        FD_SET(_sock, &read_fds);
        FD_SET(_sock, &write_fds);

        timeval timeout{1, 0};
        int ret = select(_sock+1, &read_fds, &write_fds, nullptr, &timeout);
        if (ret < 0) {
            printf("select error\n");
            return false;
        }

        if (FD_ISSET(_sock, &read_fds)) {
            FD_CLR(_sock, &read_fds);

            if (recv_data() < 0) {
                return false;
            }
        }

        return true;
    }

    bool is_run()
    {
        return _sock != INVALID_SOCKET;
    }

    int send_data(DataHeader* header)
    {
        if (is_run() && header)
        {
            return send(_sock, (const char*)header, header->data_length, 0);
        }
        return SOCKET_ERROR;
    }
private:
    int recv_data()
    {
        char buf[4096] = {};
        int len = recv(_sock, buf, sizeof(DataHeader), 0);
        DataHeader* header = (DataHeader*)buf;
        if (len < 0) {
            printf("recv data error\n", _sock);
            return -1;
        } else if (len == 0){
            return 0;
        }

        recv(_sock, buf + sizeof(DataHeader), header->data_length - sizeof(DataHeader), 0);

        on_net_msg(header);

        return header->data_length;
    }

    void on_net_msg(DataHeader* header)
    {
        switch (header->cmd) {
            case CMD_LOGIN_RESULT:
                {
                    LoginResult* loginResult = (LoginResult*)header;
                    printf("login result code:%d\n", loginResult->code);
                }
                break;
            case CMD_LOGOUT_RESULT:
                {
                    LogoutResult* logoutResult = (LogoutResult*)header;
                    printf("logout result code:%d\n", logoutResult->code);
                }
                break;
            default:
            {
                printf("unknown command\n");
            }
        }
    }

    void init_socket()
    {
        if (_sock != INVALID_SOCKET)
        {
            close();
        }
#ifdef _WIN32
        //prepare the Windows environment.
        WORD ver = MAKEWORD(2, 2);
        WSADATA data;
        WSAStartup(ver, &data);
#endif
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == _sock) {
            printf("create socket fail\n");
        }
    }

    void close_socket(SOCKET sock)
    {
        if (sock != INVALID_SOCKET)
        {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            sock = INVALID_SOCKET;
        }
    }



private:
    SOCKET _sock = INVALID_SOCKET;
};

#endif //PRACTISE_CPLUSPLUS_NET_EASY_TCP_CLIENT_HPP
