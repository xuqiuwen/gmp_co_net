#include "./include/EventLoop.h"

#include "./include/Routine.h"
#include "./include/Runtime.h"

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
  std::pair<int, IOType> tpe(fd, io_type);
  std::unique_lock lock(mtx_);
  if (event_routine_map_.find(tpe) == event_routine_map_.end()) {
    // 没找到
    event_routine_map_.insert({tpe, {}});
  }
  // 复制句柄构造一个协程包装，实际上直接使用句柄也行
  event_routine_map_[tpe].push(Routine{handle});
}
Routine EventLoop::EventDelete(std::pair<int, IOType> key) {
  std::unique_lock lock(mtx_);
  auto routine = event_routine_map_[key].front();
  event_routine_map_[key].pop();
  return routine;
}

void EventLoop::Loop() {  // 等到一个完成的事件，唤醒对应协程
  while (!shutdown_flag_.load(std::memory_order_seq_cst)) {
    auto key = async_io_.wait_for_completion();
    if (!key.has_value()) {
      continue;  // 超时用于触发shutdown_flag_
    }
    auto routine = EventDelete(key.value());
    // 调度执行，要用新接口，放到合适的M上
    Runtime::GetInstance().getScheduler().SubmitEventRoutine(routine);
  }
}

AsyncIO& EventLoop::getAsyncIO() { return async_io_; }