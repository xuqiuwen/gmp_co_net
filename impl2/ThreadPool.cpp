#include "./include/ThreadPool.h"
ThreadPool::ThreadPool(size_t thread_count, std::function<void()> func)
    : shutdown_flag_{true},
      thread_count_{thread_count},
      func_{std::move(func)} {}

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
