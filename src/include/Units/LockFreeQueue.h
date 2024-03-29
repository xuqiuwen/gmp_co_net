#pragma once
#include <atomic>
#include <cassert>
#include <memory>
#include <optional>
#include <thread>

template <typename T>
class LockFreeQueue {
 public:
  explicit LockFreeQueue(size_t capacity);
  virtual ~LockFreeQueue();
  bool Push(const T &value);
  std::optional<T> Pop();
  bool Empty() const;
  bool Full() const;

 private:
  size_t capacity_;
  std::unique_ptr<T[]> array_;
  std::atomic<size_t> size_;  // 计算真大小
  std::atomic<size_t> head_;  // 全局、单调递增的
  std::atomic<size_t> tail_;  // 全局、单调递增的
  std::atomic<size_t> max_tail_;
  size_t countToIndex(size_t index);
  bool push(const T &ele);
  bool pop(T &ele);
  size_t size() const;
};

template <typename T>
LockFreeQueue<T>::LockFreeQueue(size_t capacity)
    : capacity_{capacity}, head_{0}, tail_{0}, max_tail_{0} {
  array_ = std::make_unique<T[]>(capacity_);
}

template <typename T>
LockFreeQueue<T>::~LockFreeQueue(){};

template <typename T>
size_t LockFreeQueue<T>::countToIndex(size_t index) {
  return index % capacity_;
}

template <typename T>
size_t LockFreeQueue<T>::size() const {
  return size_.load(std::memory_order_relaxed);
};

template <typename T>
bool LockFreeQueue<T>::push(const T &ele) {
  size_t current_head;
  size_t current_tail;

  do {  // 先占据一个位置
    current_tail = tail_.load(std::memory_order_relaxed);  // 这句要在上面
    current_head = head_.load(std::memory_order_relaxed);
    // 判满
    if (countToIndex(current_tail + 1) == countToIndex(current_head)) {
      return false;
    }
    // CAS不能使用(current_tail + 1) % capacity_
  } while (!tail_.compare_exchange_weak(current_tail, current_tail + 1,
                                        std::memory_order_relaxed));

  array_[countToIndex(current_tail)] = ele;

  size_t tmp = current_tail;
  while (true) {  // 提交修改，要一个一个提交
    if (max_tail_.compare_exchange_weak(current_tail, current_tail + 1,
                                        std::memory_order_relaxed)) {
      break;
    }
    current_tail = tmp;  // 这句很关键，恢复 current_tail
    std::this_thread::yield();  // 核心数小于生产者数时，使用此句性能更好
    // webserver只有少数生产者，可以不用上面这句
  }
  size_.fetch_add(1);
  return true;
}

template <typename T>
bool LockFreeQueue<T>::pop(T &ele) {
  size_t current_head;
  size_t current_max_tail;

  do {
    current_head = head_.load(std::memory_order_relaxed);
    current_max_tail = max_tail_.load(std::memory_order_relaxed);
    // 判空
    if (countToIndex(current_head) == countToIndex(current_max_tail)) {
      return false;
    }

    ele = array_[countToIndex(current_head)];  // 重新读
    // 占据位置，说明读的正确
    if (head_.compare_exchange_weak(current_head, current_head + 1,
                                    std::memory_order_relaxed)) {
      size_.fetch_sub(1);
      return true;
    }

  } while (true);

  // 不可达
  assert(0);
  return false;
}

template <typename T>
bool LockFreeQueue<T>::Push(const T &ele) {
  return push(ele);
}

template <typename T>
std::optional<T> LockFreeQueue<T>::Pop() {
  T val;
  bool ret = pop(val);
  if (!ret) {
    return std::nullopt;
  }
  return val;
}

template <typename T>
bool LockFreeQueue<T>::Empty() const {
  return size() == 0;
}

template <typename T>
bool LockFreeQueue<T>::Full() const {
  return size() == capacity_;
}