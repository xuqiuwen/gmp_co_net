#pragma once
#include <atomic>
#include <functional>
#include <optional>
#include <thread>
#include <vector>

#include "LockFreeQueue.h"

// 线程池，管理所有的 worker
class ThreadPool {
 public:
  ThreadPool(size_t thread_count, std::function<void()> func);
  void Start();
  void Shutdown();

 private:
  std::vector<std::jthread> workers_;
  std::atomic<bool> shutdown_flag_;
  size_t thread_count_;
  std::function<void()> func_;
};