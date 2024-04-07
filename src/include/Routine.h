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
   private:
    int data_;  // 保存协程数据，比如异步读写的实际大小，定时器用不上
   public:
    void setData(int data) {
      // std::cerr << this << "setData" << data << std::endl;
      data_ = data;
    }
    int getData() {
      // std::cerr << this << "getData" << data_ << std::endl;
      return data_;
    };
    Task get_return_object() { return Task{handle_type::from_promise(*this)}; }
    auto initial_suspend() { return std::suspend_always{}; }
    std::suspend_never final_suspend() noexcept;
    void return_void() {}
    void unhandled_exception() {
      perror("unhandled_exception");
      exit(1);
    }
  };
};

// 轻量级的，包含句柄，可以直接复制，看成指针。实际上没有必要，使用句柄就绪，这样只是好看点
class Routine {
 public:
  Routine(){};
  Routine(Task task) : handler_{task.handle_} {};  // 支持隐式转换
  Routine(std::coroutine_handle<Task::promise_type> handler)
      : handler_{handler} {};  // 支持隐式转换
  void resume() {
    // debug("恢复执行一个协程");
    // std::cout << handler_.address() << std::endl;
    handler_.resume();
  }
  // 这里要返回引用
  Task::promise_type& GetPromise() { return handler_.promise(); }

 private:
  std::coroutine_handle<Task::promise_type> handler_;  // 协程句柄
};