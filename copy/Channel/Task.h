#pragma once
#include <atomic>
#include <coroutine>
#include <future>
#include <iostream>
#include <thread>
#include <utility>

#include "TotalVariable.h"

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
      uncompleted_task_count.fetch_sub(1);
      // std::cout << "当前计数" << uncompleted_task_count << std::endl;
      if (uncompleted_task_count == 0) {
        zero_cv.notify_one();
      }
      return std::suspend_never{};
    }
    void return_void() {}  // co_return 不带值
    void unhandled_exception() { std::terminate(); }
  };
};
