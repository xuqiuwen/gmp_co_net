#pragma once
#include <atomic>
#include <mutex>
class SpinLock {
 public:
  void lock() {
    while (lock_.exchange(true, std::memory_order_acquire)) {
      // 忙等待（spin）直到锁被释放
    }
  }

  void unlock() { lock_.store(false, std::memory_order_release); }

 private:
  std::atomic<bool> lock_{false};
};