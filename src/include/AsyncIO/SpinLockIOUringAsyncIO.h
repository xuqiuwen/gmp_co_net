#pragma once
#include <fcntl.h>
#include <liburing.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

#include "../Units/SpinLock.h"
#include "AsyncIO.h"

class SpinLockIOUringAsyncIO : public AsyncIO {
 public:
  SpinLockIOUringAsyncIO(size_t event_count);
  ~SpinLockIOUringAsyncIO() override;
  void async_read(int fd, char* buf, size_t nbytes) override;
  void async_write(int fd, const char* buf, size_t nbytes) override;
  std::optional<std::pair<int, IOType>> wait_for_completion(
      int& nbytes) override;

 private:
  io_uring ring;
  SpinLock mtx_;  // 保证SpinLockIOUringAsyncIO的并发的线程安全
  // 潜在的优化方向，为每个M配备一个AsyncIO，不用互斥了就
  // 哈希表也要拆分成M个，也不用互斥了就。
  // 调整窃取策略和本地队列，不需要互斥；不改策略，直接使用无锁队列
};