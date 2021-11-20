#include <thread>
#include "easy_tcp_client.hpp"

void handle_command(EasyTcpClient* easyTcpClient)
{
    while (easyTcpClient->is_run()) {
        char cmd_buf[4096] = {};
        int scan_len = scanf("%s", cmd_buf);
        if (scan_len < 0) {
            printf("scan input fail.\n");
            break;
        }

        if (0 == strcmp(cmd_buf, "exit")) {

            printf("exit loop\n");
            easyTcpClient->close();
            break;
        } else if (0 == strcmp(cmd_buf, "login")) {
            Login login;
            strcpy(login.user_name, "zhangshan");
            strcpy(login.user_pwd, "123456");
            easyTcpClient->send_data(&login);
        } else if (0 == strcmp(cmd_buf, "logout")) {
            Logout logout;
            strcpy(logout.user_name, "zhangshan");
            easyTcpClient->send_data(&logout);
        }
    }
}

int main() {
    setbuf(stdout, nullptr);
    printf("[client start...]\n");
    EasyTcpClient client;
    int ret = client.connect("127.0.0.1", 4567);
    if (SOCKET_ERROR == ret)
    {
        printf("connect server error\n");
        return -1;
    }

    std::thread t(handle_command, &client);
    t.detach();

    while (client.is_run()) {
        client.on_run();
    }

    client.close();
    return 0;
}
