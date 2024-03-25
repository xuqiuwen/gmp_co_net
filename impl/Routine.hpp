#include <coroutine>
#include <future>
#include <iostream>
#include <thread>
#include <utility>

// struct HandleAwaiterGet {  // 用于在协程内部获得句柄的等待体
//   std::coroutine_handle<> handle;
//   bool await_ready() const noexcept { return false; }
//   void await_suspend(std::coroutine_handle<> h) noexcept {
//     handle = h;
//     h.resume();
//   }
//   std::coroutine_handle<> await_resume() const noexcept { return handle; }
// };

struct Task {  // 协程返回值类
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  std::coroutine_handle<promise_type> handle_;  // 协程句柄
  Task(handle_type h) : handle_(h) {}           // 构造函数
  struct promise_type {
    Task get_return_object() { return Task{handle_type::from_promise(*this)}; }
    auto initial_suspend() {  // 构造时挂起
      return std::suspend_always{};
    }
    auto final_suspend() noexcept {  // 析构时不挂起
      return std::suspend_never{};
    }
    void return_void() {}  // co_return 不带值
    void unhandled_exception() { std::terminate(); }
  };
};
