# tcp 底层通信轮子
1. 使用操作系统提供的原生 API 搭建一个高性能的服务端框架
2. Windows 主要基于 select , iocp
3. Linux 主要基于 select, epoll

## 2021-12-04
1. 客户端测试代码变更
   1. 增加多个发送线程
   2. 增加的每一个线程同时发送多个包
2. 服务端变更成多线程
   1. 一个线程接收新客户端链接
   2. N 个线程处理客户端收、发 ![模型图片](./doc/生产者-消费者模型.png)
   3. 增加客户端计数（新连接，断开，消息数量），使用 atomic, 有竞争，暂时不用考虑，目前性能瓶颈在 select 模型上。
   

## 2021-11-21
1. tcp 协议限制
   1. 需要确认收包，只能根据接收缓冲区最小的一方来调整数据的收发频率。
   2. 在发送方和接收方都增加缓冲区，每次都尽可能多的从 tcp 缓冲区读完所有数据，保持 tcp 层最大量的数据传输。
   3. 降低系统 recv 调用次数，增强性能。
      1. 测试方式
         1. 一方只发，另一方只收，查看单独接收或者单独发送的能力。
         2. 一方一次只收一个 char，查看调用次数极限。
   4. 粘包、少包处理。
2. windows 修改 FD_SET_SIZE 适配能处理大于 1000 个链接。
3. 增加精确到毫秒级的计数器，方便统计每秒的收发包次数。

## 2021-11-20 
1. 提取消息结构体入 message_header.hpp 文件， 方便 tcp_server, tcp_client 公用。
2. 增加对 tcp client, server 的封装，应对以后同一个服务需要多一个 client, server 的情况。
3. 增加工作函数，并剥离接收数据函数和处理数据函数。
   1. 接收函数 
      1. 处理粘包问题 
   2. 处理数据函数
      1. 专门处理消息
      
