#include <coroutine>
#include <future>
#include <thread>
#include <utility>

struct Task {  // 协程返回值类
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  std::coroutine_handle<promise_type> handle_;  // 协程句柄
  Task(handle_type h) : handle_(h) {}           // 构造函数
  struct promise_type {
    Task get_return_object() { return Task{handle_type::from_promise(*this)}; }
    auto initial_suspend() {  // 构造时不挂起
      return std::suspend_never{};
    }
    auto final_suspend() noexcept {  // 析构时不挂起
      return std::suspend_never{};
    }
    void return_void() {}  // co_return 不带值
    void unhandled_exception() { std::terminate(); }
  };
};

// 一个协程函数
Task go(std::function<void()> func) {
  std::jthread(std::move(func)).detach();
  co_return;
}
