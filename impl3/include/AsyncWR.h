#pragma once
#include <sys/epoll.h>
#include <unistd.h>

#include <coroutine>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "EventLoop.h"

struct AsyncReadAwaiter {  // 异步读等待器
  int fd_;
  char *buf_;
  size_t bytes_;
  AsyncReadAwaiter(int fd, char *buf, size_t bytes)
      : fd_{fd}, buf_{buf}, bytes_{bytes} {}
  bool await_ready() const noexcept { return false; }  // 上来就挂起
  void await_suspend(std::coroutine_handle<> handle) {
    auto &l = EventLoop::GetInstance();
    // std::unique_lock<std::mutex>(l.mutex_);
    l.AddQueue(fd_, EPOLLIN, handle);  // handle加入阻塞哈希表
    l.AddOrModifyFd(fd_, EPOLLIN);     // 加入事件循环
  }
  void await_resume() { read(fd_, buf_, bytes_); }
};

struct AsyncWriteAwaiter {  // 异步写等待器
  int fd_;
  char *buf_;
  size_t bytes_;
  AsyncWriteAwaiter(int fd, char *buf, size_t bytes)
      : fd_{fd}, buf_{buf}, bytes_{bytes} {}
  bool await_ready() const noexcept { return false; }  // 上来就挂起
  void await_suspend(std::coroutine_handle<> handle) {
    auto &l = EventLoop::GetInstance();
    l.AddQueue(fd_, EPOLLOUT, handle);  // 加入阻塞哈希表
    l.AddOrModifyFd(fd_, EPOLLOUT);     // 加入事件循环
  }
  void await_resume() { write(fd_, buf_, bytes_); }
};

// 定义为内联函数
inline AsyncReadAwaiter read_async(int fd, char *buf, size_t bytes) {  // 异步读
  return AsyncReadAwaiter{fd, buf, bytes};
}

inline AsyncWriteAwaiter write_async(int fd, char *buf,
                                     size_t bytes) {  // 异步写
  return AsyncWriteAwaiter{fd, buf, bytes};
}