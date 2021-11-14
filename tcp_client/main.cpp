
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <cstdint>
#include <thread>

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

bool b_run = true;
void handle_command(SOCKET sock)
{
    while (true) {
        char cmd_buf[4096] = {};
        int scan_len = scanf("%s", cmd_buf);
        if (scan_len < 0) {
            printf("scan input fail.\n");
            break;
        }

        if (0 == strcmp(cmd_buf, "exit")) {
            b_run = false;
            printf("exit loop\n");
            break;
        } else if (0 == strcmp(cmd_buf, "login")) {
            Login login;
            strcpy(login.user_name, "zhangshan");
            strcpy(login.user_pwd, "123456");

            send(sock, (const char*)&login, sizeof(login), 0);
        } else if (0 == strcmp(cmd_buf, "logout")) {
            Logout logout;
            strcpy(logout.user_name, "zhangshan");

            send(sock, (const char*)&logout, sizeof(logout), 0);
        }
    }
}

int processor(SOCKET sock)
{
    DataHeader dh = {};
    int read_len = recv(sock, (char*)&dh, sizeof(dh), 0);
    if (read_len < 0) {
        return -1;
    }

    printf("recv data from server cmd:%d, data length:%d\n", dh.cmd, dh.data_length);

    const int header_len = sizeof(DataHeader);
    switch (dh.cmd) {
        case CMD_LOGIN_RESULT:
        {
            LoginResult loginResult = {};
            recv(sock, (char*)&loginResult+header_len, sizeof(loginResult)-header_len, 0);
            printf("login result code:%d\n", loginResult.code);
        }
        break;
        case CMD_LOGOUT_RESULT:
        {
            LogoutResult logoutResult = {};
            recv(sock, (char*)&logoutResult+header_len, sizeof(logoutResult)-header_len, 0);
            printf("logout result code:%d\n", logoutResult.code);
        }
        break;
        default:
        {
            printf("unknown command\n");
        }
    }

    return 0;
}

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

    std::thread t(handle_command, _socket);
    t.detach();

    while (b_run) {
        fd_set read_fds;
        fd_set write_fds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        FD_SET(_socket, &read_fds);
        FD_SET(_socket, &write_fds);

        timeval timeout{1, 0};
        int ret = select(_socket+1, &read_fds, &write_fds, nullptr, &timeout);
        if (ret < 0) {
            printf("select error\n");
            break;
        }

        if (FD_ISSET(_socket, &read_fds)) {
            FD_CLR(_socket, &read_fds);

            if (processor(_socket) < 0) {
                break;
            }
        }
    }

    closesocket(_socket);

    WSACleanup();
    return 0;
}
