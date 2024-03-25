#pragma once
#include <atomic>
#include <functional>
#include <optional>
#include <thread>
#include <vector>

#include "LockFreeQueue.hpp"

// 线程池，管理所有的 worker
class ThreadPool {
 public:
  ThreadPool(size_t thread_count, std::function<void()> func);
  virtual ~ThreadPool();
  void Start();
  void Shutdown();

 private:
  std::vector<std::jthread> workers_;
  std::atomic<bool> shutdown_flag_;
  size_t thread_count_;
  std::function<void()> func_;
};

ThreadPool::ThreadPool(size_t thread_count, std::function<void()> func)
    : shutdown_flag_{true},
      thread_count_{thread_count},
      func_{std::move(func)} {}

ThreadPool::~ThreadPool() {
  shutdown_flag_.store(true, std::memory_order_relaxed);
}

void ThreadPool::Start() {
  shutdown_flag_ = false;
  for (size_t i = 0; i < thread_count_; ++i) {
    workers_.emplace_back([this] {
      while (!shutdown_flag_.load(std::memory_order_relaxed)) {
        func_();
      }
    });
  }
}
void ThreadPool::Shutdown() {
  shutdown_flag_.store(true, std::memory_order_relaxed);
}
