#include <thread>
#include "easy_tcp_client.hpp"

volatile bool b_run = true;
void handle_command()
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
            break;
        }
    }
}

int main() {
    setbuf(stdout, nullptr);
    printf("[client start...]\n");
    const int n_count = 1000;
    EasyTcpClient* clients[n_count];
    for (int i = 0; i < n_count; i++) {
        clients[i] = new EasyTcpClient();
        int ret = clients[i]->connect("127.0.0.1", 4567);
        if (SOCKET_ERROR == ret)
        {
            printf("connect server error\n");
            return -1;
        }
    }


    std::thread t(handle_command);
    t.detach();

    Login login;
    strcpy(login.user_name, "zhangshan");
    strcpy(login.user_pwd, "123456");
    while (b_run) {
        for (int i = 0; i < n_count; i++) {
            clients[i]->on_run();
            clients[i]->send_data(&login);
        }
    }
    for (int i = 0; i < n_count; i++) {
        clients[i]->close();
    }
    return 0;
}
