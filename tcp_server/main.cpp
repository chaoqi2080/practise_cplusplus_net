
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

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
        char recv_buf[4096] = {};
        int recv_len = recv(client_socket, recv_buf, 4096, 0);
        if (recv_len < 0) {
            printf("client exit\n");
            break;
        }

        printf("recv data from client:%s\n", recv_buf);

        int write_len = 0;
        if (0 == strcmp(recv_buf, "get_name")) {
            char reply_buf[] = {"zhang shan"};
            write_len = send(client_socket, reply_buf, strlen(reply_buf), 0);
        } else if (0 == strcmp(recv_buf, "get_age")) {
            char reply_buf[] = {"18"};
            write_len = send(client_socket, reply_buf, strlen(reply_buf), 0);
        } else if (0 == strcmp(recv_buf, "exit")) {
            char reply_buf[] = {"exit"};
            write_len = send(client_socket, reply_buf, strlen(reply_buf), 0);
            printf("client exit\n");
            break;
        } else {
            char reply_buf[] = {"unknown command"};
            write_len = send(client_socket, reply_buf, strlen(reply_buf), 0);
        }

        printf("write %d to client\n", write_len);
    }

    //close the server socket.
    closesocket(_socket);

    //clean up the Windows environment.
    WSACleanup();

    return 0;
}
