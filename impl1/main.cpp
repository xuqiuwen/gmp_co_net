#include <coroutine>
#include <deque>
#include <functional>
#include <iostream>

class Scheduler {
 public:
  // 就绪队列中的任务类型
  using Task = std::coroutine_handle<>;

  // 将协程添加到就绪队列
  void schedule(Task task) {
    readyQueue.push_back(task);
    // 可以选择立即执行事件循环或延迟执行
  }

  // 模拟事件发生，将挂起的协程移动到就绪队列
  void resume(Task task) {
    // 此处简化处理：直接将任务加入就绪队列
    readyQueue.push_back(task);
  }

  // 事件循环
  void run() {
    while (!readyQueue.empty()) {
      auto task = readyQueue.front();
      readyQueue.pop_front();
      if (!task.done()) {
        task.resume();
      }
    }
  }

 private:
  std::deque<Task> readyQueue;
};
Scheduler scheduler;  // 全局调度器实例

struct Awaitable {
  bool await_ready() const noexcept { return false; }  // 总是挂起
  void await_suspend(std::coroutine_handle<> handle) {
    // 在此处挂起协程，并将控制权返回给调度器
    std::cout << "被挂起了" << std::endl;
    scheduler.resume(handle);  // 假设`scheduler`是全局可访问的调度器
  }
  void await_resume() const noexcept {}

  // 添加与事件相关的成员和方法，以支持不同的挂起条件
};

struct Task {
  struct promise_type {
    Task get_return_object() {
      return {std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() { std::terminate(); }
  };

  std::coroutine_handle<> handle;
  Task(std::coroutine_handle<> h) : handle(h) {}
  ~Task() {
    if (handle) handle.destroy();
  }
};

Task exampleCoroutine() {
  co_await Awaitable{};
  std::cout << "Coroutine resumed" << std::endl;
}

int main() {
  auto task1 = exampleCoroutine();
  auto task2 = exampleCoroutine();
  scheduler.schedule(task1.handle);  // 将协程添加到就绪队列
  scheduler.schedule(task2.handle);  // 将协程添加到就绪队列
  scheduler.run();  // 执行事件循环，处理就绪队列中的协程

  return 0;
}
