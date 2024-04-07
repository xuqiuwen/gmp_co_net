# 基于GMP模型实现的协程网络框架
关键技术：C++20、linux io_uring、事件驱动、协程、并发编程
>代码菜鸡的菜鸡代码，未经过严格测试，慎用
## 简要介绍
结合C++20的协程特性和linux的io_uring接口，实现支持高效的异步I/O和任务调度的网络框架，为I/O密集型应用提供一个高效的异步执行环境。
* 无锁并发数据结构：实现无锁队列并发数据结构管理协程和事件
* 异步I/O模型：实现基于io_uring和事件循环的异步I/O处理模型
* 协程调度与管理：借鉴Golang的GMP并发模型，实现M:N协程调度器
* 协程间通信机制：实现基于Channel的协程间通信机制
* CPU负载均衡：实现协程窃取机制
* 外部接口：仿照Golang的语言特性，实现用户友好的协程接口
## 一个示例
利用库编写了一个简易的回声服务器示例
```shell
make pack ;生成libgoroutine库
make server ;生成 server.out
make client ;生成 client.out
./server.out 8888 ;运行 server.out
./client.out 8888 ;运行 client.out
```
## 使用方法
### 引入头文件
```C++
#include "LibGoRoutine.h"
```
### 定义了一些宏/函数
| 宏定义/函数         | 描述                                                           | 使用示例                         |
| ------------------- | -------------------------------------------------------------- | -------------------------------- |
| `GO_START`          | 启动运行时，用于初始化协程环境。                               | `GO_START`                       |
| `GO(func)`          | 启动一个新的协程。                                             | `GO(myRoutine)`                  |
| `GO_WRITE(a, b, c)` | 协程写操作，将数据写入指定的文件描述符。返回值为实际写出大小。 | `GO_WRITE(fd, buffer, size)`     |
| `GO_READ(a, b, c)`  | 协程读操作，从指定的文件描述符读取数据。返回值为实际读入大小。 | `GO_READ(fd, buffer, size)`      |
| `GO_CHANNEL`        | 定义一个协程间通信的通道。                                     | `GO_CHANNEL<int> foo;`           |
| `foo.write(bar)`    | 协程间通信的通道的写操作。                                     | `co_await foo.write(1234)`       |
| `foo.read()`        | 协程间通信的通道的读操作，返回值为读到的值 。                  | `auto bar = co_await foo.read()` |
## 模块间依赖关系
![image](https://github.com/xuqiuwen/gmp_co_net/assets/84625276/16e308f3-67ec-4d0d-9072-8bb00a62b3eb)
## 待添加功能
* memory_order_seq_cst改为更松的限制
* 底层可选epoll作为事件监听机制，作为对照
* 调整窃取策略和本地队列的调度方法，调整阻塞后协程调度策略
* 为每achine配备一个AsyncIO，相应的事件哈希映射也要拆分每个machine一个
## 性能优化
针对 echo_server 进行性能优化
### 优化方向
有两个地方使用了 mutex 互斥，事件循环的事件映射哈希表 和 io_uring 线程安全的互斥访问
* 事件映射：无锁哈希表、自旋锁，锁分离技术
* io_uring：批提交、自旋锁
### 优化结果
| Machine数量 | 事件循环 | io_uring | QPS     |
| ----------- | -------- | -------- | ------- |
| 2           | SpinLock | 不优化   | 25193.3 |
| 4           | SpinLock | 不优化   | 18611.8 |
| 2           | 锁分离   | 不优化   | 23372.7 |
| 4           | 锁分离   | 不优化   | 18611.8 |
| 2           | 不优化   | SpinLock | 47041   |
| 4           | 不优化   | SpinLock | 30855.1 |
| 2           | SpinLock | SpinLock | 48620.2 |
| 4           | SpinLock | SpinLock | 31322.3 |
| 2           | 锁分离   | SpinLock | 48143.6 |
| 4           | 锁分离   | SpinLock | 30270.8 |