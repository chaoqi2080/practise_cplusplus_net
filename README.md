# tcp 底层通信轮子
1. 使用操作系统提供的原生 API 搭建一个高性能的服务端框架
2. Windows 主要基于 select , iocp
3. Linux 主要基于 select, epoll


## 2021-11-20 
1. 提取消息结构体入 message_header.hpp 文件， 方便 tcp_server, tcp_client 公用。
2. 增加对 tcp client, server 的封装，应对以后同一个服务需要多一个 client, server 的情况。
3. 增加工作函数，并剥离接收数据函数和处理数据函数。
   1. 接收函数 
      1. 处理粘包问题 
   2. 处理数据函数
      1. 专门处理消息
      
