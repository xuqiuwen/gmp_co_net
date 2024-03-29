#include "../include/AsyncIO/EpollAsyncIO.h"

#include "../include/RoutineIO.h"
EpollAsyncIO::EpollAsyncIO(size_t event_count) : event_count_(event_count) {
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
  }
}

EpollAsyncIO::~EpollAsyncIO() { close(epoll_fd); }

void EpollAsyncIO::async_read(int fd, char* buf, size_t count) {
  operations[fd] = {buf, count, Operation::Read};
  epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
    perror("epoll_ctl: add");
    exit(EXIT_FAILURE);
  }
}

void EpollAsyncIO::async_write(int fd, const char* buf, size_t count) {
  operations[fd] = {const_cast<char*>(buf), count, Operation::Write};
  epoll_event event;
  event.events = EPOLLOUT;
  event.data.fd = fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
    perror("epoll_ctl: add");
    exit(EXIT_FAILURE);
  }
}

// 待修改
std::optional<std::pair<int, IOType>> EpollAsyncIO::wait_for_completion(
    int& nbytes) {
  return std::nullopt;
  const int event_count_ = 10;
  epoll_event events[event_count_];
  int n = epoll_wait(epoll_fd, events, event_count_, -1);
  for (int i = 0; i < n; i++) {
    int fd = events[i].data.fd;
    auto& op = operations[fd];
    if ((events[i].events & EPOLLIN) && op.type == Operation::Read) {
      read(fd, op.buf, op.count);
    } else if ((events[i].events & EPOLLOUT) && op.type == Operation::Write) {
      write(fd, op.buf, op.count);
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
  }
  return std::pair{0, IOType::Read};
}
