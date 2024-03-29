#pragma once
#include <mutex>
#include <optional>
#include <queue>

template <typename T>
class MutexSafeQueue {
 public:
  explicit MutexSafeQueue(size_t max_size) : max_size_{max_size} {}

  bool Push(const T& value) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.size() == max_size_) {
      return false;
    }
    queue_.push(value);
    return true;
  }

  std::optional<T> Pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return {};
    }
    // debug("pop");

    T value = queue_.front();
    queue_.pop();
    return value;
  }

  bool Empty() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  bool Full() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size() == max_size_;
  }

 private:
  size_t max_size_;
  std::queue<T> queue_;
  mutable std::mutex mutex_;
};
