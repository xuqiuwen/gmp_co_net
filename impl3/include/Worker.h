#pragma once
#include <atomic>
#include <coroutine>
#include <thread>

#include "LockFreeQueue.h"
#include "TotalVariable.h"

class Worker {
 public:
  Worker() : shutdown_flag_(false), local_task_queues_(local_queue_size) {}
  void Start();
  void Shutdown();

 private:
  void work();

  std::atomic<bool> shutdown_flag_;
  std::jthread worker_thread_;
  LockFreeQueue<std::coroutine_handle<>> local_task_queues_;
};
