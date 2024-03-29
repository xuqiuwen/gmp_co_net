#pragma once
#include <fcntl.h>
#include <liburing.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

#include "AsyncIO.h"

class IOUringAsyncIO : public AsyncIO {
 public:
  IOUringAsyncIO(size_t event_count);
  ~IOUringAsyncIO() override;
  void async_read(int fd, char* buf, size_t nbytes) override;
  void async_write(int fd, const char* buf, size_t nbytes) override;
  std::optional<std::pair<int, IOType>> wait_for_completion() override;

 private:
  io_uring ring;
  std::mutex mtx_;  // 保证并发的线程安全
  // 潜在的优化方向，为每个M配备一个AsyncIO，实现无锁
  // 哈希表也要拆分成M个，也无锁
  // 本地队列也不需要互斥
};