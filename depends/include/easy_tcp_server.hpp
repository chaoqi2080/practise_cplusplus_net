//
// Created by chaoqi on 2021/11/20.
//

#ifndef PRACTISE_CPLUSPLUS_NET_EASY_TCP_SERVER_HPP
#define PRACTISE_CPLUSPLUS_NET_EASY_TCP_SERVER_HPP
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
    #include <unistd.h>
#endif

#include "message_header.hpp"

class EasyTcpServer
{
public:
    EasyTcpServer()
    {

    }

    ~EasyTcpServer()
    {
        close();
    }

    int listen(const char* ip, unsigned short port, int backlog = 64)
    {
        bind(ip, port);

        int ret = ::listen(_sock, backlog);
        if (SOCKET_ERROR == ret) {
            printf("listen _socket error\n");
        }

        return ret;
    }

    int on_run()
    {
        fd_set read_fds;
        fd_set write_fds;
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        SOCKET max_socket = _sock;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        //
        FD_SET(_sock, &read_fds);
        FD_SET (_sock, &write_fds);

        for (auto itr = _clients.begin(); itr != _clients.end(); ++itr) {
            SOCKET cur = *itr;

            FD_SET(cur, &read_fds);
            FD_SET (cur, &write_fds);

            if (max_socket < cur)
            {
                max_socket = cur;
            }
        }

        int nfds = select(max_socket+1, &read_fds, &write_fds, nullptr, &timeout);
        if (nfds < 0) {
            printf("select return < 0, break\n");
            return nfds;
        }

        if (FD_ISSET(_sock, &read_fds)) {
            FD_CLR(_sock, &read_fds);

            accept();
        }

        handle_recv(read_fds);

        handle_write(write_fds);
    }

    void close()
    {
        //close all client socket.
        for (auto itr = _clients.begin(); itr != _clients.end(); ++itr) {
            close_socket(*itr);
        }

        _clients.clear();

        //close the server socket.
        close_socket(_sock);
#ifdef _WIN32
        //clean up the Windows environment.
        WSACleanup();
#endif
    }

    bool is_run()
    {
        return _sock != INVALID_SOCKET;
    }

private:
    int bind(const char* ip, unsigned short port)
    {
        if (_sock == INVALID_SOCKET)
        {
            init_socket();
        }
        sockaddr_in sock_in = {};
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = htons(port);
#ifdef _WIN32
        sock_in.sin_addr.S_un.S_addr = ip == nullptr ? INADDR_ANY : inet_addr(ip);
#else
        sock_in.sin_addr.s_addr = ip == nullptr ? INADDR_ANY : inet_addr(ip);
#endif
        int ret = ::bind(_sock, (sockaddr*)&sock_in, sizeof(sock_in));
        if (SOCKET_ERROR == ret) {
            printf("bind _socket error\n");
        }

        return ret;
    }

    int send_data(SOCKET sock, DataHeader* header)
    {
        if (is_run() && sock != INVALID_SOCKET && header)
        {
            return send(sock, (const char*)header, header->data_length, 0);
        }
        return SOCKET_ERROR;
    }

    void send2all(DataHeader* header)
    {
        for (auto itr = _clients.begin(); itr != _clients.end(); ++itr)
        {
            send_data(*itr, header);
        }
    }

    void handle_recv(const fd_set& read_fds)
    {
        for (auto itr = _clients.begin(); itr != _clients.end();) {
            SOCKET cur_socket = *itr;

            if (FD_ISSET(cur_socket, &read_fds)) {
                FD_CLR(cur_socket, &read_fds);

                if (recv_data(cur_socket) < 0) {
                    close_socket(cur_socket);
                    itr = _clients.erase(itr);
                    continue;
                }
            }

            ++itr;
        }
    }

    void handle_write(const fd_set& write_fds)
    {
        for (auto itr = _clients.begin(); itr != _clients.end();) {
            SOCKET cur_socket = *itr;

            if (FD_ISSET(cur_socket, &write_fds)) {
                FD_CLR(cur_socket, &write_fds);

//                if (send_data(cur_socket) < 0) {
//                    close_socket(cur_socket);
//                    itr = _clients.erase(itr);
//                    continue;
//                }
            }

            ++itr;
        }
    }

    int accept()
    {
        sockaddr_in client_addr = {};
        unsigned int client_addr_len = sizeof(client_addr);

        SOCKET new_socket = INVALID_SOCKET;
        new_socket = ::accept(_sock, (sockaddr*)&client_addr, (int*)&client_addr_len);
        if (INVALID_SOCKET == new_socket) {
            printf("accept _socket error\n");
            return new_socket;
        }

        NewUserJoin newUserJoin = {};
        newUserJoin.user_id = new_socket;
        send2all(&newUserJoin);

        _clients.push_back(new_socket);
#ifdef _WIN32
        printf("new client join socket:%llu, ip:%s\n", new_socket, inet_ntoa(client_addr.sin_addr));
#else
        printf("new client join socket:%d, ip:%s\n", new_socket, inet_ntoa(client_addr.sin_addr));
#endif
    }

    int recv_data(SOCKET socket)
    {
        char buf[4096] = {};
        int len = recv(socket, (char*)buf, sizeof(DataHeader), 0);
        DataHeader* header = (DataHeader*)buf;
        if (len < 0)
        {
            printf("read data header fail.\n");
            return -1;
        }

        printf("recv data from client, cmd:%d, data len:%d\n", header->cmd, header->data_length);

        recv(socket, (char*)buf + sizeof(DataHeader), header->data_length - sizeof(DataHeader), 0);

        on_net_msg(socket, header);

        return header->data_length;
    }

    int on_net_msg(SOCKET sock, DataHeader* header)
    {
        switch (header->cmd) {
            case CMD_LOGIN:
            {
                Login* login = (Login*)header;
                printf("user login name:%s, pwd:%s\n", login->user_name, login->user_pwd);

                LoginResult loginResult;
                loginResult.code = 0;
                send(sock, (const char*)&loginResult, sizeof(loginResult), 0);
            }
                break;
            case CMD_LOGOUT:
            {
                Logout* logout = (Logout*)header;
                printf("user logout name:%s\n", logout->user_name);

                LogoutResult logoutResult;
                logoutResult.code = 0;
                send(sock, (const char*)&logoutResult, sizeof(logoutResult), 0);
            }
                break;
            default:
            {
                DataHeader dh;
                send(sock, (const char*)&dh, sizeof(dh), 0);
                printf("unknown command\n");
            }
        }

        return 0;
    }

    void init_socket()
    {
        if (_sock != INVALID_SOCKET)
        {
            return;
        }
#ifdef _WIN32
        //prepare the Windows environment.
        WORD ver = MAKEWORD(2, 2);
        WSADATA data;
        WSAStartup(ver, &data);
#endif
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    std::vector<SOCKET> _clients;
};

#endif //PRACTISE_CPLUSPLUS_NET_EASY_TCP_SERVER_HPP
