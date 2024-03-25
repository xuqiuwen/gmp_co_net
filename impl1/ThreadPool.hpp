#pragma once
#include <atomic>
#include <functional>
#include <optional>
#include <thread>
#include <vector>

#include "LockFreeQueue.hpp"

class ThreadPool {
 public:
  explicit ThreadPool(size_t thread_count, size_t task_count);
  virtual ~ThreadPool();
  template <typename F>  // 使用std::function<void()>也可以，但是性能不好
  void Submit(F&& task);  // 从外部向线程池提交任务，task是可调用对象
  size_t ThreadCount() const;
  size_t TaskCount() const;
  void Shutdown();

 private:
  std::vector<std::jthread> workers_;
  LockFreeQueue<std::shared_ptr<std::function<void()>>> tasks_;
  std::atomic<bool> shutdown_flag_;
};

ThreadPool::ThreadPool(size_t thread_count, size_t task_count)
    : shutdown_flag_{false}, tasks_(1000) {  // 任务队列大小
  for (size_t i = 0; i < thread_count; ++i) {
    workers_.emplace_back([this] {
      while (!shutdown_flag_.load(std::memory_order_relaxed)) {  // 检查
        std::shared_ptr<std::function<void()>> task;
        bool has = tasks_.Pop(task);
        if (has) {  // 有任务
          (*task)();
        } else {  // 没任务，让出时间片，针对任务不密集的场景
          std::this_thread::yield();
        }
      }  // 检查失败线程就结束了
    });
  }
}

ThreadPool::~ThreadPool() {
  // 关闭线程池，不然线程不会结束，永远join不到
  shutdown_flag_.store(true, std::memory_order_relaxed);
  // jthread不用join，自动join并析构
}

template <typename F>
void ThreadPool::Submit(F&& task) {
  tasks_.Push(std::make_shared<std::function<void()>>(std::forward<F>(task)));
}

size_t ThreadPool::TaskCount() const { return tasks_.Size(); }

size_t ThreadPool::ThreadCount() const { return workers_.size(); }

void ThreadPool::Shutdown() {  // 任务队列没开始的任务就不管了
  shutdown_flag_.store(true, std::memory_order_relaxed);
  workers_.clear();
  // jthread析构时会自动join线程，并且该线程也会阻塞等待
}
