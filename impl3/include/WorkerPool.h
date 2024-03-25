#pragma once
#include <atomic>
#include <functional>
#include <optional>
#include <thread>
#include <vector>

#include "LockFreeQueue.h"
#include "Worker.h"
// 线程池，管理所有的 worker
class WorkerPool {
 public:
  friend class Scheduler;
  WorkerPool(size_t thread_count);
  void Start();
  void Shutdown();

 private:
  std::vector<Worker> workers_;
  std::atomic<bool> shutdown_flag_;
  size_t thread_count_;
};