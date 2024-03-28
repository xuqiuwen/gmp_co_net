#include "./include/EventLoop.h"

#include "./include/Routine.h"

EventLoop::EventLoop(size_t loop_size)
    : shutdown_flag_(true), async_io_(loop_size) {}
void EventLoop::Start() {
  shutdown_flag_.store(false, std::memory_order_seq_cst);
  loop_thread_ = std::move(std::jthread([&] { Loop(); }));
}
void EventLoop::Stop() {
  shutdown_flag_.store(true, std::memory_order_seq_cst);
}
void EventLoop::EventRegiste(int fd, IOType io_type,
                             std::coroutine_handle<> handle) {
  std::pair tpe(fd, io_type);
  if (event_routine_map_.find(tpe) == event_routine_map_.end()) {
    // 没找到
    event_routine_map_.insert({tpe, {}});
  }
  // 复制句柄构造一个协程包装，实际上直接使用句柄也行
  event_routine_map_[tpe].push(Routine{handle});
}

void EventLoop::Loop() {  // 等到一个完成的事件，唤醒对应协程
  while (!shutdown_flag_.load(std::memory_order_seq_cst)) {
    auto a = async_io_.wait_for_completion();
    event_routine_map_[a].front().resume();
    event_routine_map_[a].pop();
  }
}

AsyncIO &EventLoop::getAsyncIO() { return async_io_; }