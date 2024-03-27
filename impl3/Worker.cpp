#include "./include/Worker.h"

#include "./include/Scheduler.h"

void Worker::Start() {
  worker_thread_ = std::jthread([this] {
    while (!shutdown_flag_.load(std::memory_order_relaxed)) {
      work();
    }
  });
}

void Worker::Shutdown() {
  shutdown_flag_.store(true, std::memory_order_relaxed);  // 通知线程停止
}

void Worker::work() {
  // 从对应的工作队列选取协程，如果没有从全局队列选取
  std::coroutine_handle<> task;
  auto has = local_task_queues_.Pop(task);
  if (has) {        // 本地有任务
    task.resume();  // 唤醒协程并执行
  } else {          // 本地没有任务
    bool has = Scheduler::GetInstance().global_task_queue_.Pop(task);
    if (has) {        // 全局有任务
      task.resume();  // 唤醒协程并执行
    } else {  // 没任务，针对任务不密集的场景，可以让出时间片
      std::this_thread::yield();
    }
  }
}
