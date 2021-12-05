#include <thread>
#include "easy_tcp_client.hpp"

volatile bool b_run = true;
//客户端的数量
const int n_count = 10000;
std::shared_ptr<EasyTcpClient> clients[n_count];
//开启线程数量
const int t_count = 4;

const int num_per_thread = n_count / t_count;

void sender(int id)
{
    int start = (id - 1) * num_per_thread;
    int end = id * num_per_thread;

    printf("sender id:%d start:%d, end:%d\n", id, start, end);

    for (int i = start; i < end; i++) {
        clients[i] = std::make_shared<EasyTcpClient>();
    }

    for (int i = start; i < end; i++) {
        int ret = clients[i]->connect("127.0.0.1", 4567);
        if (SOCKET_ERROR == ret)
        {
            printf("i:%d, connect server error\n", i);
            return;
        }
    }

    //
    std::this_thread::sleep_for(5s);

    Login login;
    strcpy(login.user_name, "zhangshan");
    strcpy(login.user_pwd, "123456");
    while (b_run) {
        for (int i = start; i < end; i++) {
            clients[i]->on_run();
            clients[i]->send_data(&login);
        }
    }

    for (int i = start; i < end; i++) {
        clients[i]->close();
    }
}

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
    //setbuf(stdout, nullptr);
    printf("[client start...]\n");

    for (int i = 1; i <= t_count; i++) {
        std::thread t(sender, i);
        t.detach();
    }

    std::thread t(handle_command);
    t.detach();

    while (true) {
        //printf("[client test end...]\n");
    }

    return 0;
}
