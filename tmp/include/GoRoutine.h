#pragma once
#include <atomic>
#include <coroutine>
#include <future>
#include <iostream>
#include <thread>
#include <utility>

// #include "Scheduler.h"
#include "TotalVariable.h"
#include "io_utils.h"
struct Task {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  std::coroutine_handle<promise_type> handle_;
  Task(handle_type h) : handle_(h) {}
  struct promise_type {
    Task get_return_object() { return Task{handle_type::from_promise(*this)}; }
    auto initial_suspend() { return std::suspend_always{}; }
    auto final_suspend() noexcept {
      // Scheduler::GetInstance().CompleteRoutine();
      return std::suspend_never{};
    }
    void return_void() {}
    void unhandled_exception() { exit(1); }
  };
};

// 轻量级的，包含句柄，可以直接复制，看成指针
class GoRoutine {
 public:
  explicit GoRoutine(Task task) : handler_{task.handle_} {};
  GoRoutine(std::coroutine_handle<> handler) : handler_{handler} {};
  void resume() {
    // debug("恢复一个协程");
    // std::cout << handler_.address() << std::endl;
    handler_.resume();
  }

 private:
  std::coroutine_handle<> handler_;  // 协程句柄
};