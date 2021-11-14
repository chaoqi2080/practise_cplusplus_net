
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
    printf("[client start...]\n");
    //prepare the Windows environment.
    WORD ver = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(ver, &data);

    SOCKET _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _socket) {
        printf("create socket fail\n");
        return -1;
    }

    sockaddr_in sock_in = {};
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(4567);
    sock_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_socket, (sockaddr*)&sock_in, sizeof(sock_in));
    if (SOCKET_ERROR == ret) {
        printf("connect server error\n");
        return ret;
    }

    while (true) {
        char cmd_buf[4096] = {};
        int scan_len = scanf("%s", cmd_buf);

        if (0 == strcmp(cmd_buf, "exit")) {
            printf("exit loop\n");
            break;
        } else if (0 == strcmp(cmd_buf, "login")) {
            Login login;
            strcpy(login.user_name, "zhangshan");
            strcpy(login.user_pwd, "123456");

            send(_socket, (const char*)&login, sizeof(login), 0);
        } else if (0 == strcmp(cmd_buf, "logout")) {
            Logout logout;
            strcpy(logout.user_name, "zhangshan");

            send(_socket, (const char*)&logout, sizeof(logout), 0);
        }

        DataHeader dh = {};
        int read_len = recv(_socket, (char*)&dh, sizeof(dh), 0);

        printf("recv data from server cmd:%d, data length:%d\n", dh.cmd, dh.data_length);
        switch (dh.cmd) {
            case CMD_LOGIN_RESULT:
            {
                LoginResult loginResult = {};
                recv(_socket, (char*)&loginResult, sizeof(loginResult), 0);
                printf("login result code:%d\n", loginResult.code);
            }
            break;
            case CMD_LOGOUT_RESULT:
            {
                LogoutResult logoutResult = {};
                recv(_socket, (char*)&logoutResult, sizeof(logoutResult), 0);
                printf("logout result code:%d\n", logoutResult.code);
            }
            break;
            default:
            {
                printf("unknown command\n");
            }
        }

    }

    closesocket(_socket);

    WSACleanup();
    return 0;
}
