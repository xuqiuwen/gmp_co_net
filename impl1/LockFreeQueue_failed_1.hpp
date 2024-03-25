#pragma once

#include <atomic>
#include <cstddef>
#include <memory>

template <typename T>
class LockFreeQueue {  // 基于循环数组的无锁队列
 public:
  explicit LockFreeQueue(size_t capacity);

  bool push(const T &ele);  // 入队列
  bool pop(T &ele);         // 出队列
  size_t size() const;      // 返回大小

 private:
  size_t capacity_;             // 最大容量
  std::unique_ptr<T[]> array_;  // 数组
  std::atomic<size_t> head_;    // 头
  std::atomic<size_t> tail_;    // 尾
};

template <typename T>
LockFreeQueue<T>::LockFreeQueue(size_t capacity) : capacity_(capacity) {
  array_ = std::make_unique<T[]>(capacity_ + 1);  // 数组长度等于最大容量+1
}

// 1. 获取尾，可能会同时获取尾
// 2. 获取的地方添加元素，竞争！
// 3. 尾巴后移

// 1. 只允许一个线程获取尾巴，原子的获取尾
// 2. 再把尾加1，(1,2)要是原子的<=>只有一个线程能把同一个尾加1，并且占有它
// 但是没有合适的命令，只能通过两个指令结合的方式
template <typename T>
bool LockFreeQueue<T>::push(const T &ele) {
  T *node;
  size_t tmp = tail_.load(std::memory_order_relaxed);  // 原子获得队列 tail
  while (true) {
    if (size() == capacity_) {
      return false;
    }
    if ((tail_.compare_exchange_weak(
            tmp, (tmp + 1) % capacity_,
            std::
                memory_order_relaxed))) {  // CAS操作，获得的tmp是否是要操作的tail
      break;                               // 成功就占用它，tail加1
    }  // 失败说明有其他线程成功占有，会把tmp更新为最新的tail
  }  // 不断重试
  // 成功占有
  array_[tmp] = ele;  // 复制元素
  return true;
}

template <typename T>
bool LockFreeQueue<T>::pop(T &ele) {
  T *node;
  size_t tmp = head_.load(std::memory_order_relaxed);  // 原子获得队列 head
  while (true) {
    if (size() == 0) {
      return false;
    }
    if ((head_.compare_exchange_weak(
            tmp, (tmp + 1) % capacity_,
            std::
                memory_order_relaxed))) {  // CAS操作，获得的tmp是否是要操作的head
      break;                               // 成功就占用它，head加1
    }  // 失败说明有其他线程成功占有，会把tmp更新为最新的head
  }                   // 不断重试
  ele = array_[tmp];  // 成功占有， 复制元素
  return true;
}

template <typename T>
size_t LockFreeQueue<T>::size() const {
  size_t head = head_.load(std::memory_order_relaxed);
  size_t tail = tail_.load(std::memory_order_relaxed);
  if (tail >= head) {
    return tail - head;
  } else {
    return capacity_ + 1 - head + tail;
  }
}