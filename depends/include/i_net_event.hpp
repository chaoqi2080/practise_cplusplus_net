//
// Created by chaoqi on 2021/12/5.
//

#ifndef PRACTISE_CPLUSPLUS_NET_I_NET_EVENT_HPP
#define PRACTISE_CPLUSPLUS_NET_I_NET_EVENT_HPP

#include "cell_net_utils.hpp"
#include "tcp_client_s.hpp"

class INetEvent
{
public:
    virtual void on_user_leave(std::shared_ptr<TcpClientS>) = 0;
    virtual void on_net_message(std::shared_ptr<TcpClientS>) = 0;
};

#endif //PRACTISE_CPLUSPLUS_NET_I_NET_EVENT_HPP
