//
// Created by chaoqi on 2021/11/20.
//

#ifndef PRACTISE_CPLUSPLUS_NET_EASY_TCP_SERVER_HPP
#define PRACTISE_CPLUSPLUS_NET_EASY_TCP_SERVER_HPP

#include "message_header.hpp"
#include "tcp_client_s.hpp"
#include "cell_net_utils.hpp"
#include "cell_timestamp.hpp"
#include "cell_server.hpp"

class EasyTcpServer
{
public:
    EasyTcpServer() = default;

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

        printf("listen on port<%d> ok\n", port);

        return ret;
    }

    void start()
    {
        if (!is_run()) {
            printf("listen must call before start.\n");
            return;
        }

        for (int i = 0; i < CELL_THREAD_COUNT; i++) {
            auto cell_server = std::make_shared<CellServer>(_sock);
            _cell_servers.push_back(cell_server);
            //启动收、发工作任务
            cell_server->start();
        }
    }

    int on_run()
    {
        fd_set read_fds;
        fd_set write_fds;

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        //
        FD_SET(_sock, &read_fds);
        FD_SET (_sock, &write_fds);

        int nfds = select(_sock + 1, &read_fds, &write_fds, nullptr, &timeout);
        if (nfds < 0) {
            printf("select return < 0, break\n");
            return nfds;
        }

        if (FD_ISSET(_sock, &read_fds)) {
            FD_CLR(_sock, &read_fds);
            accept();
        }

        return 0;
    }

    void close()
    {
        close_socket(_sock);
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

        add_client2cell_server(new_socket);
        return 0;
    }

    void add_client2cell_server(SOCKET sock)
    {
        auto min_cell_server = _cell_servers[0];
        for (auto cell_server : _cell_servers) {
            if (min_cell_server->get_client_count() > cell_server->get_client_count()) {
                min_cell_server = cell_server;
            }
        }

        min_cell_server->add_client(std::make_shared<TcpClientS>(sock));
    }

    void init_socket()
    {
        if (_sock != INVALID_SOCKET)
        {
            return;
        }

        static NetEnv obj;
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }

private:
    SOCKET _sock = INVALID_SOCKET;
    //CellTimestamp _timer;
    //uint32_t _recv_count = 0;
    std::vector<std::shared_ptr<CellServer>> _cell_servers;
};

#endif //PRACTISE_CPLUSPLUS_NET_EASY_TCP_SERVER_HPP
