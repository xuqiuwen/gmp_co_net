#include <sys/epoll.h>
#include <unistd.h>

#include <coroutine>
#include <memory>
#include <stdexcept>
#include <unordered_map>

class EventLoop {
 public:
  EventLoop(int max_size);  // 可监听事件数目
  ~EventLoop();
  int AddFd(int fd, uint32_t events);  // 注册事件
  int DeleteFd(int fd);                // 删除事件
  void WaitResume();  // 等到了一个事件，唤醒对应协程
  void AddFdQueue(int fd, uint32_t events, std::coroutine_handle<> handle);

 private:
  int epoll_fd;                                 // epoll的文件描述符
  int max_size;                                 // 事件最大个数
  std::unique_ptr<epoll_event[]> epoll_events;  // epoll事件数组
  std::unordered_map<int,
                     std::unordered_multimap<uint32_t, std::coroutine_handle<>>>
      block_coroutines_;  // 阻塞协程
};

EventLoop::EventLoop(int max_size) {
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    exit(1);
  }

  epoll_events = std::make_unique<epoll_event[]>(max_size);
}

EventLoop::~EventLoop() {}

int EventLoop::AddFd(int fd, uint32_t event) {
  epoll_event ep_event;
  ep_event.events = event;
  ep_event.data.fd = fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ep_event) == -1) {
    return -1;
  }
  return 0;
}

// 加入阻塞队列
void EventLoop::AddFdQueue(int fd, uint32_t event,
                           std::coroutine_handle<> handle) {
  block_coroutines_[fd].insert({event, handle});
}

int EventLoop::DeleteFd(int fd) {
  if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return -1;
  }
  return 0;
}

void EventLoop::WaitResume() {
  while (true) {
    int event_count = epoll_wait(epoll_fd, epoll_events.get(), max_size, -1);
    for (int i = 0; i < event_count; i++) {
      int fd = epoll_events[i].data.fd;
      uint32_t event = epoll_events[i].events;
      auto ite1 = block_coroutines_.find(fd);  // 外层哈希表迭代器
      if (ite1 == block_coroutines_.end()) {
        return;
      }
      auto ite2 = ite1->second.find(event);
      if (ite2 == ite1->second.end()) {  // 内层哈希表迭代器
        return;
      }
      auto handler = ite2->second;
      ite1->second.erase(ite2);    // 从内层哈希表移除事件
      handler.resume();            // 唤醒该阻塞协程
      if (ite1->second.empty()) {  // 该fs上没有任何协程就不监听
        block_coroutines_.erase(ite1);  // 移除fd哈希表
        DeleteFd(fd);
      }
    }
  }
}

struct AsyncReadAwaiter {
  int fd_;
  char *buf_;
  size_t bytes_;
  EventLoop *event_loop_;
  AsyncReadAwaiter(int fd, char *buf, size_t bytes, EventLoop *event_loop)
      : fd_{fd}, buf_{buf}, bytes_{bytes}, event_loop_{event_loop} {}
  bool await_ready() const noexcept { return false; }  // 上来就挂起
  void await_suspend(std::coroutine_handle<> handle) {
    event_loop_->AddFd(fd_, EPOLLIN);               // 加入事件循环
    event_loop_->AddFdQueue(fd_, EPOLLIN, handle);  // 加入阻塞哈希表
  }

  void await_resume() { read(fd_, buf_, bytes_); }
};

void write_async(int fd) {  // 异步写
}
auto read_async(int fd, char *buf, size_t bytes,
                EventLoop *event_loop) {  // 异步读
  return AsyncReadAwaiter{fd, buf, bytes, event_loop};
}