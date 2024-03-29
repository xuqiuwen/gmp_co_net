#pragma once
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

#include "AsyncIO.h"

class EpollAsyncIO : public AsyncIO {
 public:
  EpollAsyncIO(size_t event_count);
  ~EpollAsyncIO() override;
  void async_read(int fd, char* buf, size_t count) override;
  void async_write(int fd, const char* buf, size_t count) override;
  std::optional<std::pair<int, IOType>> wait_for_completion(
      int& nbytes) override;

 private:
  int epoll_fd;
  size_t event_count_;
  enum class Operation { Read, Write };
  struct Op {
    char* buf;
    size_t count;
    Operation type;
  };
  std::map<int, Op> operations;
};
