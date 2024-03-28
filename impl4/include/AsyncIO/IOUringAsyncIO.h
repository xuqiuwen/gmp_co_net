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
  std::pair<int, IOType> wait_for_completion() override;

 private:
  io_uring ring;
};