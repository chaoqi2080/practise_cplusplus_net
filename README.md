# tcp 底层通信轮子
1. 使用操作系统提供的原生 API 搭建一个高性能的服务端框架
2. Windows 主要基于 select , iocp
3. Linux 主要基于 select, epoll

## 2021-11-21
1. tcp 协议限制
   1. 需要确认收包，只能根据接收缓冲区最小的一方来调整数据的收发频率。
   2. 在发送方和接收方都增加缓冲区，每次都尽可能多的从 tcp 缓冲区读完所有数据，保持 tcp 层最大量的数据传输。
   3. 降低系统 recv 调用次数，增强性能。
   4. 粘包、少包处理。

## 2021-11-20 
1. 提取消息结构体入 message_header.hpp 文件， 方便 tcp_server, tcp_client 公用。
2. 增加对 tcp client, server 的封装，应对以后同一个服务需要多一个 client, server 的情况。
3. 增加工作函数，并剥离接收数据函数和处理数据函数。
   1. 接收函数 
      1. 处理粘包问题 
   2. 处理数据函数
      1. 专门处理消息
      
