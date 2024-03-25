#pragma once
#include <sys/epoll.h>
#include <unistd.h>

#include <coroutine>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "Scheduler.h"
#include "TotalVariable.h"
class EventLoop {
 public:
  static EventLoop &GetInstance();

  // 确保不能复制或移动单例对象
  EventLoop(const EventLoop &) = delete;
  EventLoop &operator=(const EventLoop &) = delete;
  EventLoop(EventLoop &&) = delete;
  EventLoop &operator=(EventLoop &&) = delete;

  int AddOrModifyFd(int fd, uint32_t events);  // 注册事件
  int DeleteFd(int fd);                        // 删除事件
  void Run();                                  // 启动事件循环
  void Shutdown();                             // 关闭循环
  void AddQueue(int fd, uint32_t events,
                std::coroutine_handle<> handle);  // 注册阻塞队列

 private:
  EventLoop();  // 私有构造
  ~EventLoop();
  std::atomic<bool> shutdown_flag_;
  int epoll_fd_;                                // epoll的文件描述符
  int loop_size_;                               // 事件最大个数
  std::unique_ptr<epoll_event[]> epoll_events;  // epoll事件数组
  std::unordered_map<int,
                     std::unordered_multimap<uint32_t, std::coroutine_handle<>>>
      block_coroutines_hash_;  // 阻塞协程哈希表
  std::mutex hash_mutex_;      // 保护共享的 block_coroutines_hash_
};
