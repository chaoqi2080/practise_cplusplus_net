//
// Created by chaoqi on 2021/11/20.
//

#ifndef PRACTISE_CPLUSPLUS_NET_EASY_TCP_CLIENT_HPP
#define PRACTISE_CPLUSPLUS_NET_EASY_TCP_CLIENT_HPP

#include "message_header.hpp"
#include "cell_net_utils.hpp"

class EasyTcpClient
{
public:
    EasyTcpClient() = default;

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
        } else if (ret == 0) {
            return true;
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
        //每次从 tcp 缓冲区尽可能多的读取数据
        char buf[4096] = {};
        int len = recv(_sock, buf, 4096, 0);
        if (len <= 0) {
            printf("<%d> recv data error, len:%d\n", (int)_sock, len);
            return len;
        }

        if (_last_recv_pos + len > RECV_BUF_SIZE) {
            printf("error, recv buf is not enough.\n");
            return -1;
        }

        memcpy(_recv_buf + _last_recv_pos, buf, len);
        _last_recv_pos += len;

        //是否满足一个消息头长度
        while (_last_recv_pos >= sizeof(DataHeader)){
            DataHeader* header = (DataHeader*)_recv_buf;
            //是否满足一个消息长度
            if (_last_recv_pos >= header->data_length) {
                uint32_t msg_len = header->data_length;

                on_net_msg(header);

                _last_recv_pos -= msg_len;
                if (_last_recv_pos > 0) {
                    memcpy(_recv_buf, _recv_buf + msg_len, _last_recv_pos);
                }
            } else{
                break;
            }
        }

        return 0;
    }

    void on_net_msg(DataHeader* header)
    {
        switch (header->cmd) {
            case CMD_LOGIN_RESULT:
            {
                LoginResult* loginResult = (LoginResult*)header;
                //printf("login result code:%d\n", loginResult->code);
            }
                break;
            case CMD_LOGOUT_RESULT:
            {
                LogoutResult* logout_result = (LogoutResult*)header;
                //printf("logout result code:%d\n", logout_result->code);
            }
                break;
            case CMD_NEW_USER_JOIN:
            {
                NewUserJoin* user_join = (NewUserJoin*)header;
            }
                break;
            case CMD_ERROR:
            {
                printf("CMD_ERROR\n");
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
private:
    SOCKET _sock = INVALID_SOCKET;
    char _recv_buf[RECV_BUF_SIZE] = {};
    uint32_t _last_recv_pos = 0;
};

#endif //PRACTISE_CPLUSPLUS_NET_EASY_TCP_CLIENT_HPP
