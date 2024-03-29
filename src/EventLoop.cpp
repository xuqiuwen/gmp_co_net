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
                             std::coroutine_handle<Task::promise_type> handle) {
  std::pair<int, IOType> key(fd, io_type);
  event_routine_map_.push(key, Routine{handle});
}
Routine EventLoop::EventDelete(std::pair<int, IOType> key) {
  auto routine = event_routine_map_.pop(key);
  if (!routine.has_value()) {
    perror("EventDelete");
  }
  return routine.value();
}

void EventLoop::Loop() {  // 等到一个完成的事件，唤醒对应协程
  while (!shutdown_flag_.load(std::memory_order_seq_cst)) {
    int nbytes = -1;
    auto key = async_io_.wait_for_completion(nbytes);
    if (!key.has_value()) {
      continue;  // 超时返回用于触发shutdown_flag_
    }
    auto routine = EventDelete(key.value());
    routine.GetPromise().setData(nbytes);
    // std::cerr << "wait_for,实际读入" << nbytes << std::endl;
    //  调度执行，要用新接口，放到合适的M上
    Runtime::GetInstance().getScheduler().SubmitEventRoutine(routine);
  }
}

IOUringAsyncIO& EventLoop::getAsyncIO() { return async_io_; }