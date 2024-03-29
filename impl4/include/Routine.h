#pragma once
#include <atomic>
#include <coroutine>
#include <future>
#include <iostream>
#include <thread>
#include <utility>

struct Task {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  std::coroutine_handle<promise_type> handle_;
  Task(handle_type h) : handle_(h) {}
  struct promise_type {
    Task get_return_object() { return Task{handle_type::from_promise(*this)}; }
    auto initial_suspend() { return std::suspend_always{}; }
    std::suspend_never final_suspend() noexcept;
    void return_void() {}
    void unhandled_exception() { exit(1); }
  };
};

// 轻量级的，包含句柄，可以直接复制，看成指针。实际上没有必要，使用句柄就绪，这样只是好看点
class Routine {
 public:
  explicit Routine(Task task) : handler_{task.handle_} {};
  explicit Routine(std::coroutine_handle<> handler) : handler_{handler} {};
  void resume() {
    // debug("恢复执行一个协程");
    // std::cout << handler_.address() << std::endl;
    handler_.resume();
  }

 private:
  std::coroutine_handle<> handler_;  // 协程句柄
};