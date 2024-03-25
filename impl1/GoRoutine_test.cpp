#include "GoRoutine.hpp"

#include <coroutine>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
// Task and go function definitions as provided above...

int main() {
  // 启动第一个异步任务
  go([]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Task 1 executed on thread " << std::this_thread::get_id()
              << std::endl;
  });

  // 启动第二个异步任务
  go([]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Task 2 executed on thread " << std::this_thread::get_id()
              << std::endl;
  });

  // 主线程等待一段时间以观察异步任务的执行结果
  std::cout << "Main thread " << std::this_thread::get_id() << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "Main thread exit" << std::endl;

  return 0;
}
