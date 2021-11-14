
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

#include <stdio.h>
#include <cstdint>
#include <vector>



enum CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,

    CMD_ERROR,
};

struct DataHeader
{
    DataHeader()
    {
        cmd = CMD_ERROR;
        data_length = sizeof(DataHeader);
    }
    uint16_t cmd;
    uint16_t data_length;
};

struct Login : public DataHeader
{
    Login()
    {
        cmd = CMD_LOGIN;
        data_length = sizeof(Login);
    }
    char user_name[64];
    char user_pwd[64];
};

struct LoginResult : public DataHeader
{
    LoginResult()
    {
        cmd = CMD_LOGIN_RESULT;
        data_length = sizeof(LoginResult);
    }
    uint16_t code;
};

struct Logout : public DataHeader
{
    Logout()
    {
        cmd = CMD_LOGOUT;
        data_length = sizeof(Logout);
    }
    char user_name[64];
};

struct LogoutResult : public DataHeader
{
    LogoutResult()
    {
        cmd = CMD_LOGOUT_RESULT;
        data_length = sizeof(LogoutResult);
    }
    uint16_t code;
};

struct NewUserJoin : public DataHeader
{
    NewUserJoin()
    {
        cmd = CMD_NEW_USER_JOIN;
        data_length = sizeof(NewUserJoin);
    }

    uint32_t user_id;
};

std::vector<SOCKET> _clients;

int processor(SOCKET sock)
{
    DataHeader dh = {};
    int recv_len = recv(sock, (char*)&dh, sizeof(dh), 0);
    if (recv_len < 0)
    {
        printf("read data header fail.\n");
        return -1;
    }

    printf("recv data from client, cmd:%d, data len:%d\n", dh.cmd, dh.data_length);
    const int header_len = sizeof(DataHeader);
    switch (dh.cmd) {
        case CMD_LOGIN:
        {
            Login login = {};
            recv(sock, (char*)&login+header_len , sizeof(login)-header_len, 0);
            printf("user login name:%s, pwd:%s\n", login.user_name, login.user_pwd);

            LoginResult loginResult;
            loginResult.code = 0;
            send(sock, (const char*)&loginResult, sizeof(loginResult), 0);
        }
            break;
        case CMD_LOGOUT:
        {
            Logout logout = {};
            recv(sock, (char*)&logout+header_len, sizeof(logout)-header_len, 0);
            printf("user logout name:%s\n", logout.user_name);

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

void close_socket(SOCKET sock)
{
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

int main() {
    printf("[server start...]\n");
#ifdef _WIN32
    //prepare the Windows environment.
    WORD ver = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(ver, &data);
#endif
    SOCKET _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sock_in = {};
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(4567);
#ifdef _WIN32
    sock_in.sin_addr.S_un.S_addr = INADDR_ANY;
#else
    sock_in.sin_addr.s_addr = INADDR_ANY;
#endif
    int ret = bind(_socket, (sockaddr*)&sock_in, sizeof(sock_in));
    if (SOCKET_ERROR == ret) {
        printf("bind _socket error\n");
        return ret;
    }

    ret = listen(_socket, 64);
    if (SOCKET_ERROR == ret) {
        printf("listen _socket error\n");
        return ret;
    }

    _clients.clear();
    //wait for new client connect
    while (true) {
        fd_set read_fds;
        fd_set write_fds;
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        //
        FD_SET(_socket, &read_fds);
        FD_SET (_socket, &write_fds);

        for (auto itr = _clients.begin(); itr != _clients.end(); ++itr) {
            FD_SET(*itr, &read_fds);
            FD_SET (*itr, &write_fds);
        }

        int nfds = select(_socket+1, &read_fds, &write_fds, nullptr, nullptr);
        if (nfds < 0) {
            printf("select return < 0, break\n");
            break;
        }

        if (FD_ISSET(_socket, &read_fds)) {
            FD_CLR(_socket, &read_fds);

            sockaddr_in client_addr = {};
            unsigned int client_addr_len = sizeof(client_addr);

            SOCKET client_socket = INVALID_SOCKET;
            client_socket = accept(_socket, (sockaddr*)&client_addr, (int*)&client_addr_len);
            if (INVALID_SOCKET == client_socket) {
                printf("accept _socket error\n");
                return ret;
            }

            NewUserJoin newUserJoin = {};
            newUserJoin.user_id = client_socket;
            for (auto itr = _clients.begin(); itr != _clients.end(); ++itr) {
                send(*itr, (const char*)&newUserJoin, sizeof(newUserJoin), 0);
            }

            _clients.push_back(client_socket);
#ifdef _WIN32
            printf("new client join socket:%llu, ip:%s\n", client_socket, inet_ntoa(client_addr.sin_addr));
#else
            printf("new client join socket:%d, ip:%s\n", client_socket, inet_ntoa(client_addr.sin_addr));
#endif
        }

        for (auto itr = _clients.begin(); itr != _clients.end();) {
            SOCKET cur_socket = *itr;

            if (FD_ISSET(cur_socket, &read_fds)) {
                FD_CLR(cur_socket, &read_fds);

                if (processor(cur_socket) < 0) {
                    close_socket(cur_socket);
                    itr = _clients.erase(itr);
                    continue;
                }
            }

            ++itr;
        }
    }

    //close all client socket.
    for (auto itr = _clients.begin(); itr != _clients.end(); ++itr) {
        close_socket(*itr);
    }

    //close the server socket.
    close_socket(_socket);
#ifdef _WIN32
    //clean up the Windows environment.
    WSACleanup();
#endif
    return 0;
}
