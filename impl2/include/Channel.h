#pragma once
#include <atomic>
#include <coroutine>
#include <deque>
#include <exception>
#include <list>
#include <mutex>
#include <queue>

#include "TotalVariable.h"
template <typename ValueType>
struct Channel;

template <typename ValueType>
struct WriterAwaiter {
  Channel<ValueType> *channel_;
  // 要写入 Channel 的值
  ValueType value_;

  // 往 channel 里写一个value
  WriterAwaiter(Channel<ValueType> *channel, ValueType value)
      : channel_(channel), value_(value) {}

  bool await_ready() {  // 根据缓冲区大小而定
    // if (channel_->shutdown_flag_.load(std::memory_order_relaxed)) {
    //   perror("管道已经关闭, 不能写");
    // }
    if (channel_->buffer.size() ==
        channel_->buffer_capacity) {  // 缓冲区满了要挂起
      return false;
    }
    return true;
  }

  auto await_suspend(std::coroutine_handle<> handle) {
    std::unique_lock lock(channel_->channel_lock);  // 加锁
    channel_->writer_list.push_back(handle);  // 写者加入写阻塞队列
  }

  void await_resume() {
    std::unique_lock lock(channel_->channel_lock);  // 加锁
    auto &reader_list = channel_->reader_list;
    auto &buffer = channel_->buffer;
    // 未满，直接写到buffer
    buffer.push(value_);
    // 如果有挂起的读者，那么唤醒它
    if (!reader_list.empty()) {
      auto reader = reader_list.front();
      reader_list.pop_front();
      lock.unlock();                       // 恢复前要解锁
      auto &s = Scheduler::GetInstance();  // 调度
      s.Schedule(reader);
      // reader.resume();
    } else {
      lock.unlock();
    }
  }
};

template <typename ValueType>
struct ReaderAwaiter {
  Channel<ValueType> *channel_;
  // 用于 channel >> received; 这种情况

  explicit ReaderAwaiter(Channel<ValueType> *channel) : channel_(channel) {}

  bool await_ready() {
    // if (channel_->shutdown_flag_.load(std::memory_order_relaxed)) {
    //   perror("管道已经关闭, 不能读");
    // }
    // 为空，阻塞
    if (channel_->buffer.empty()) {
      return false;
    }
    return true;
  }

  auto await_suspend(std::coroutine_handle<> handle) {
    std::unique_lock lock(channel_->channel_lock);  // 加锁
    channel_->reader_list.push_back(handle);        // 读者加入阻塞队列
  }

  ValueType await_resume() {
    std::unique_lock lock(channel_->channel_lock);  // 加锁
    auto &writer_list = channel_->writer_list;
    auto &buffer = channel_->buffer;
    // 非空，直接从buffer读
    auto value = buffer.front();
    buffer.pop();
    // 如果有挂起的写者，那么唤醒它
    if (!writer_list.empty()) {
      auto writer = writer_list.front();
      writer_list.pop_front();
      lock.unlock();
      auto &s = Scheduler::GetInstance();  // 调度
      s.Schedule(writer);
      // writer.resume();
    } else {
      lock.unlock();
    }
    return value;
  }
};

template <typename ValueType>
// 实现有缓冲通道
class Channel {
 public:
  friend class WriterAwaiter<ValueType>;
  friend class ReaderAwaiter<ValueType>;
  // 构造函数，设置 Channel 大小，默认大小为 64
  explicit Channel(int capacity = channel_size) : buffer_capacity(capacity) {
    // shutdown_flag_.store(false, std::memory_order_relaxed);
  }
  // 析构函数
  ~Channel() {
    // shutdown_flag_.store(true, std::memory_order_relaxed);
  }
  // 防止 Channel 被移动或者复制
  Channel(Channel &&channel) = delete;
  Channel(Channel &) = delete;
  Channel &operator=(Channel &) = delete;

  // 关闭管道
  void close() {
    // shutdown_flag_.store(true, std::memory_order_relaxed);
  }

  // 管道写
  auto write(ValueType value) { return WriterAwaiter<ValueType>(this, value); }

  // 管道读
  auto read() { return ReaderAwaiter<ValueType>(this); }

 private:
  int buffer_capacity;                             // buffer 的容量
  std::queue<ValueType> buffer;                    // 缓冲区
  std::list<std::coroutine_handle<>> writer_list;  // 写入者挂起队列
  std::list<std::coroutine_handle<>> reader_list;  // 读取者挂起队列
  std::mutex channel_lock;                         // 锁
  // std::atomic<bool> shutdown_flag_;                // 关闭标志
};