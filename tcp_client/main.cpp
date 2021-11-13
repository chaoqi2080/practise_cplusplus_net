
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

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
        char write_buf[4096] = {};
        int scan_len = scanf("%s", write_buf);
        if (scan_len > 0) {
            send(_socket, write_buf, strlen(write_buf), 0);
        }

        char buf[4096] = {};
        int read_len = recv(_socket, buf, 4096, 0);
        if (read_len > 0){
            printf("read data :%s\n", buf);
        }

        if (0 == strcmp(buf, "exit")) {
            printf("exit loop\n");
            break;
        }
    }

    closesocket(_socket);

    WSACleanup();
    return 0;
}
