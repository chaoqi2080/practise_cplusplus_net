//
// Created by chaoqi on 2021/11/21.
//

#ifndef PRACTISE_CPLUSPLUS_NET_TCP_CLIENT_S_HPP
#define PRACTISE_CPLUSPLUS_NET_TCP_CLIENT_S_HPP

#include "cell_net_utils.hpp"

class TcpClientS
{
public:
    TcpClientS(SOCKET sock)
    {
        _sock_fd = sock;
    }

    ~TcpClientS() = default;

    char* buf()
    {
        return _recv_buf;
    }

    void set_last_recv_pos(uint32_t pos)
    {
        _last_recv_pos = pos;
    }

    uint32_t get_last_recv_pos()
    {
        return _last_recv_pos;
    }

    SOCKET sock_fd()
    {
        return _sock_fd;
    }

    uint32_t capacity()
    {
        return RECV_BUF_SIZE * 20;
    }

private:
    SOCKET _sock_fd = INVALID_SOCKET;
    char _recv_buf[RECV_BUF_SIZE * 20] = {};
    uint32_t _last_recv_pos = 0;
};

#endif //PRACTISE_CPLUSPLUS_NET_TCP_CLIENT_S_HPP
