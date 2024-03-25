#include "./include/WorkerPool.h"
WorkerPool::WorkerPool(size_t thread_count)
    : shutdown_flag_{true}, thread_count_{thread_count} {}

void WorkerPool::Start() {
  shutdown_flag_ = false;
  for (size_t i = 0; i < thread_count_; ++i) {
    workers_[i].Start();
  }
}
void WorkerPool::Shutdown() {
  shutdown_flag_.store(true, std::memory_order_relaxed);
}
