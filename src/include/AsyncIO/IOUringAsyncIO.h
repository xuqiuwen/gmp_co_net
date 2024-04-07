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
  void async_time(int time_out_fd, long long second, long long nano_second);
  std::optional<std::pair<int, IOType>> wait_for_completion(
      int& nbytes) override;

 private:
  io_uring ring;
  std::mutex mtx_;  // 保证IOUringAsyncIO的并发的线程安全
  // 潜在的优化方向，为每个M配备一个AsyncIO，不用互斥了就
  // 哈希表也要拆分成M个，也不用互斥了就。
  // 调整窃取策略和本地队列，不需要互斥；不改策略，直接使用无锁队列
};