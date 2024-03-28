#pragma once
#include <atomic>
#include <coroutine>
#include <queue>
#include <thread>
#include <unordered_map>

// 类里有示例才用include
#include "./AsyncIO/IOUringAsyncIO.h"
#include "Routine.h"

namespace std {
template <>
struct hash<pair<int, IOType>> {
  size_t operator()(const pair<int, IOType> &p) const noexcept {
    auto hash1 = std::hash<int>{}(p.first);
    auto hash2 = std::hash<int>{}(static_cast<int>(p.second));
    return hash1 ^ (hash2 << 1);
  }
};
}  // namespace std

class EventLoop {
 public:
  EventLoop(size_t loop_size);
  void Start();
  void Stop();
  void EventRegiste(int fd, IOType io_type, std::coroutine_handle<> handle);
  AsyncIO &getAsyncIO();

 private:
  std::atomic<bool> shutdown_flag_;
  std::jthread loop_thread_;
  // 可以替换为Eopll
  IOUringAsyncIO async_io_;
  // 等待事件队列，要保证线程安全，FIFO
  std::unordered_map<std::pair<int, IOType>, std::queue<Routine>>
      event_routine_map_;
  void Loop();
};