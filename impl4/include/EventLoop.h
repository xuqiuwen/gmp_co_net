#include <atomic>
#include <coroutine>
#include <queue>
#include <thread>
#include <unordered_map>

#include "./include/Routine.h"

enum class IOType;

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
  EventLoop() : shutdown_flag_(true) {}
  void Start() {
    shutdown_flag_.store(false, std::memory_order_seq_cst);
    loop_thread_ = std::move(std::jthread([&] { Loop(); }));
  }
  void Stop() { shutdown_flag_.store(true, std::memory_order_seq_cst); }
  void EventRegiste(int fd, IOType io_type, std::coroutine_handle<> handle) {
    std::pair tpe(fd, io_type);
    if (event_routine_map_.find(tpe) == event_routine_map_.end()) {
      // 没找到
      event_routine_map_.insert(tpe, {});
    }
    // 复制句柄构造一个协程包装，实际上直接使用句柄也行
    event_routine_map_[tpe].push(Routine{handle});
  }

 private:
  std::atomic<bool> shutdown_flag_;
  std::jthread loop_thread_;
  void Loop() {  // 等到一个完成的事件，唤醒对应协程
    while (!shutdown_flag_.load(std::memory_order_seq_cst)) {
      auto a = asyncio_.wait_for_completion();
      event_routine_map_[a].front().resume();
      event_routine_map_[a].pop();
    }
  }
  // 等待事件队列，要保证线程安全，FIFO
  std::unordered_map<std::pair<int, IOType>, std::queue<Routine>>
      event_routine_map_;
};