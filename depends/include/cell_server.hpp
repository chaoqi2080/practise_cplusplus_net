//
// Created by chaoqi on 2021/12/4.
//

#ifndef PRACTISE_CPLUSPLUS_NET_CELL_SERVER_HPP
#define PRACTISE_CPLUSPLUS_NET_CELL_SERVER_HPP

#include "cell_net_utils.hpp"
#include "i_net_event.hpp"

class CellServer
{
public:
    CellServer(SOCKET sock = INVALID_SOCKET)
    {
        _sock = sock;
        _clients.clear();
        _clients_buff.clear();
    }

    ~CellServer()
    {
        _sock = INVALID_SOCKET;
        _clients.clear();
        _clients_buff.clear();
    }

    void set_net_obj(std::shared_ptr<INetEvent*> p)
    {
        _parent = p;
    }

    size_t get_client_count()
    {
        return _clients.size() + _clients_buff.size();
    }

    void add_client(std::shared_ptr<TcpClientS> client)
    {
        std::lock_guard<std::mutex> lg(_mutex);
        _clients_buff.push_back(client);
    }

    void start()
    {
        _pthread = std::make_shared<std::thread>(std::mem_fun(&CellServer::on_run), this);
        _pthread->detach();
    }

    void on_run()
    {
        while (is_run()) {
            //把新客户端加入处理队列
            if (!_clients_buff.empty()) {
                std::lock_guard<std::mutex> lg(_mutex);
                for (auto client : _clients_buff) {
                    _clients.push_back(client);
                }
                _clients_buff.clear();
            }

            if (_clients.empty()) {
                std::this_thread::sleep_for(1ms);
                continue;
            }

            fd_set read_fds;
            fd_set write_fds;
            timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);

            SOCKET max_socket = _clients[0]->sock_fd();

            for (auto itr = _clients.begin(); itr != _clients.end(); ++itr) {

                SOCKET cur_fd = (*itr)->sock_fd();

                FD_SET(cur_fd, &read_fds);
                FD_SET (cur_fd, &write_fds);
                if (max_socket < cur_fd)
                {
                    max_socket = cur_fd;
                }
            }

            int nfds = select(max_socket+1, &read_fds, &write_fds, nullptr, &timeout);
            if (nfds < 0) {
                printf("CellServer select return < 0, break\n");
                return;
            }

            handle_recv(read_fds);

            //handle_write(write_fds);
        }
    }

    int recv_data(std::shared_ptr<TcpClientS> clientS)
    {
        char buf[4096] = {};
        int len = recv(clientS->sock_fd(), (char*)buf, 4096, 0);
        //printf("len:%d\n", len);
        if (len <= 0) {
            printf("<%d> recv_data error\n", (int)clientS->sock_fd());
            return -1;
        }

        uint32_t last_pos = clientS->get_last_recv_pos();
        if (last_pos + len > clientS->capacity()) {
            printf("<%d> out of buffer error\n", (int)clientS->sock_fd());
            return -1;
        }

        //
        memcpy(clientS->buf() + last_pos, buf, len);
        clientS->set_last_recv_pos(last_pos + len);

        while (clientS->get_last_recv_pos() >= sizeof(DataHeader)) {
            DataHeader* header = (DataHeader*)clientS->buf();

            if (clientS->get_last_recv_pos() >= header->data_length) {
                uint32_t msg_len = header->data_length;
                uint32_t left_msg_len = clientS->get_last_recv_pos() - msg_len;

                on_net_msg(clientS, header);

                if (left_msg_len > 0) {
                    memcpy(clientS->buf(), clientS->buf() + msg_len, left_msg_len);
                }

                clientS->set_last_recv_pos(left_msg_len);

            } else {
                break;
            }
        }

        return 0;
    }

    void handle_recv(const fd_set& read_fds)
    {
        for (auto itr = _clients.begin(); itr != _clients.end();) {
            auto client = *itr;

            if (FD_ISSET(client->sock_fd(), &read_fds)) {
                FD_CLR(client->sock_fd(), &read_fds);

                if (recv_data(client) < 0) {
                    if (_parent) {
                        (*_parent)->on_user_leave(client);
                    }

                    close_socket(client->sock_fd());
                    itr = _clients.erase(itr);
                    continue;
                }
            }
            ++itr;
        }
    }

    int on_net_msg(std::shared_ptr<TcpClientS> clientS, DataHeader* header)
    {
        _msg_count++;
//        if (_parent) {
//            (*_parent)->on_net_message(clientS);
//        }

        switch (header->cmd) {
            case CMD_LOGIN:
            {
                Login* login = (Login*)header;
                //printf("user login name:%s, pwd:%s\n", login->user_name, login->user_pwd);

                //msg_count++;

                LoginResult loginResult;
                loginResult.code = 0;
                send_data(clientS->sock_fd(), &loginResult);
            }
                break;
            case CMD_LOGOUT:
            {
                Logout* logout = (Logout*)header;
                //printf("user logout name:%s\n", logout->user_name);

                //msg_count++;

                LogoutResult logoutResult;
                logoutResult.code = 0;
                send_data(clientS->sock_fd(), &logoutResult);
            }
                break;
            default:
            {
                printf("unknown command\n");
            }
        }

        //printf("handle msg count %d\n", msg_count);

        return 0;
    }

    bool is_run()
    {
        return _sock != INVALID_SOCKET;
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
            send_data((*itr)->sock_fd(), header);
        }
    }

    int get_msg_count()
    {
        return (int)_msg_count;
    }

    void reset_msg_count()
    {
        _msg_count = 0;
    }

private:
    SOCKET _sock = INVALID_SOCKET;
    //当前工作的客户端
    std::vector<std::shared_ptr<TcpClientS>> _clients;
    //新加入的客户端
    std::vector<std::shared_ptr<TcpClientS>> _clients_buff;
    //锁定新加入的客户端
    std::mutex _mutex;
    //工作子线程
    std::shared_ptr<std::thread> _pthread = nullptr;

    std::shared_ptr<INetEvent*> _parent = nullptr;

    std::atomic_int32_t _msg_count{0};
};

#endif //PRACTISE_CPLUSPLUS_NET_CELL_SERVER_HPP
