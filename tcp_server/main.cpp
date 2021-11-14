
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <cstdint>

enum CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,

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

int main() {
    printf("[server start...]\n");
    //prepare the Windows environment.
    WORD ver = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(ver, &data);

    SOCKET _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sock_in = {};
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(4567);
    sock_in.sin_addr.S_un.S_addr = INADDR_ANY;
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

    //wait for new client connect
    sockaddr_in client_addr = {};
    int client_addr_len = sizeof(client_addr);

    SOCKET client_socket = INVALID_SOCKET;
    client_socket = accept(_socket, (sockaddr*)&client_addr, &client_addr_len);
    if (INVALID_SOCKET == client_socket) {
        printf("accept _socket error\n");
        return ret;
    }
    printf("new client join socket:%llu, ip:%s\n", client_socket, inet_ntoa(client_addr.sin_addr));

    while (true) {
        DataHeader dh = {};
        int recv_len = recv(client_socket, (char*)&dh, sizeof(dh), 0);
        if (recv_len < 0)
        {
            printf("read data header fail.\n");
            break;
        }

        printf("recv data from client, cmd:%d, data len:%d\n", dh.cmd, dh.data_length);
        const int header_len = sizeof(DataHeader);
        switch (dh.cmd) {
            case CMD_LOGIN:
            {
                Login login = {};
                recv(client_socket, (char*)&login+header_len , sizeof(login)-header_len, 0);
                printf("user login name:%s, pwd:%s\n", login.user_name, login.user_pwd);

                LoginResult loginResult;
                loginResult.code = 0;
                send(client_socket, (const char*)&loginResult, sizeof(loginResult), 0);
            }
            break;
            case CMD_LOGOUT:
            {
                Logout logout = {};
                recv(client_socket, (char*)&logout+header_len, sizeof(logout)-header_len, 0);
                printf("user logout name:%s\n", logout.user_name);

                LogoutResult logoutResult;
                logoutResult.code = 0;
                send(client_socket, (const char*)&logoutResult, sizeof(logoutResult), 0);
            }
            break;
            default:
            {
                DataHeader dh;
                send(client_socket, (const char*)&dh, sizeof(dh), 0);
                printf("unknown command\n");
            }
        }
    }

    //close the server socket.
    closesocket(_socket);

    //clean up the Windows environment.
    WSACleanup();

    return 0;
}
