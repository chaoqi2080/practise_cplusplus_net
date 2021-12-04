


#include <stdio.h>
#include <vector>

#include "easy_tcp_server.hpp"


int main() {
    setbuf(stdout, nullptr);
    printf("[server start...]\n");
    EasyTcpServer server;
    server.listen(nullptr, 4567);
    server.start();

    //wait for new client connect
    while (server.is_run()) {
        server.on_run();
    }

    server.close();

    return 0;
}
