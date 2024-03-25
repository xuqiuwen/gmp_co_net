#include "./include/EventLoop.h"
EventLoop &EventLoop::GetInstance() {
  static EventLoop instance;
  return instance;
}

EventLoop::EventLoop() : shutdown_flag_{true}, loop_size_(loop_size) {
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    exit(1);
  }

  epoll_events = std::make_unique<epoll_event[]>(loop_size);
}

EventLoop::~EventLoop() { Shutdown(); }

int EventLoop::AddOrModifyFd(int fd, uint32_t event) {
  epoll_event ep_event;
  ep_event.events = event;
  ep_event.data.fd = fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ep_event) == -1) {
    return -1;
  }
  return 0;
}

// 加入阻塞队列
void EventLoop::AddQueue(int fd, uint32_t event,
                         std::coroutine_handle<> handle) {
  std::unique_lock<std::mutex> lock(hash_mutex_);  // 加锁要命名，不要搞错了
  block_coroutines_hash_[fd].insert({event, handle});
}

int EventLoop::DeleteFd(int fd) {
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return -1;
  }
  return 0;
}

void EventLoop::Run() {
  // std::this_thread::sleep_for(std::chrono::microseconds(1));
  shutdown_flag_ = false;
  while (!shutdown_flag_.load(std::memory_order_relaxed)) {
    // 用阻塞需要虚假唤醒不然无法结束，非阻塞不用
    int event_count = epoll_wait(epoll_fd_, epoll_events.get(), loop_size_, 0);
    for (int i = 0; i < event_count; i++) {
      std::unique_lock<std::mutex> lock(hash_mutex_);  // 互斥访问哈希表
      int fd = epoll_events[i].data.fd;
      uint32_t event = epoll_events[i].events;
      auto ite1 = block_coroutines_hash_.find(fd);  // 外层哈希表迭代器
      if (ite1 == block_coroutines_hash_.end()) {
        exit(1);
      }
      auto ite2 = ite1->second.find(event);
      if (ite2 == ite1->second.end()) {  // 内层哈希表迭代器
        exit(1);
      }

      auto handler = ite2->second;
      auto &s = Scheduler::GetInstance();  // 该句柄加入就绪队列
      s.Schedule(handler);

      ite1->second.erase(ite2);    // 从内层哈希表移除事件
      if (ite1->second.empty()) {  // 该fs上没有任何协程就不监听
        block_coroutines_hash_.erase(ite1);  // 移除fd哈希表
        // 如果要持续监听，为了减少系统调用，可以推迟到close时清理
        DeleteFd(fd);
        // std::cout << "成功清除fd" << fd << std::endl;
      }
    }
  }
}

void EventLoop::Shutdown() {
  // while (1) {  // 直到没有阻塞的协程任务
  //   if (block_coroutines_hash_.empty()) {
  //     break;
  //   }
  // }
  shutdown_flag_.store(true, std::memory_order_relaxed);
}