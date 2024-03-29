#pragma once
#include <atomic>
#include <coroutine>
#include <deque>
#include <exception>
#include <list>
#include <mutex>
#include <queue>

#include "Runtime.h"
#include "TotalVariable.h"

template <typename T>
struct Channel;

template <typename T>
struct WriterAwaiter {
  Channel<T> *channel_;
  // 要写入 Channel 的值
  T value_;
  // 往 channel 里写一个value
  WriterAwaiter(Channel<T> *channel, T value)
      : channel_(channel), value_(value) {}

  bool await_ready() {  // 根据缓冲区大小而定
    if (channel_->buffer_.size() ==
        channel_->buffer_capacity_) {  // 缓冲区满了要挂起
      return false;
    }
    return true;
  }

  auto await_suspend(std::coroutine_handle<Task::promise_type> handle) {
    std::unique_lock lock(channel_->channel_lock_);  // 加锁
    channel_->writer_list_.push_back(handle);  // 写者加入写阻塞队列
  }

  void await_resume() {
    std::unique_lock lock(channel_->channel_lock_);  // 加锁
    auto &reader_list = channel_->reader_list_;
    auto &buffer = channel_->buffer_;
    // 未满，直接写到buffer
    buffer.push(value_);
    // 如果有挂起的读者，那么唤醒它
    if (!reader_list.empty()) {
      auto reader = reader_list.front();
      reader_list.pop_front();
      lock.unlock();  // 恢复前要解锁
      Runtime::GetInstance().getScheduler().SubmitEventRoutine(reader);
    } else {
      lock.unlock();
    }
  }
};

template <typename T>
struct ReaderAwaiter {
  Channel<T> *channel_;
  // 用于 channel >> received; 这种情况

  explicit ReaderAwaiter(Channel<T> *channel) : channel_(channel) {}

  bool await_ready() {
    if (channel_->buffer_.empty()) {
      return false;
    }
    return true;
  }

  auto await_suspend(std::coroutine_handle<Task::promise_type> handle) {
    std::unique_lock lock(channel_->channel_lock_);  // 加锁
    channel_->reader_list_.push_back(handle);        // 读者加入阻塞队列
  }

  T await_resume() {
    std::unique_lock lock(channel_->channel_lock_);  // 加锁
    auto &writer_list = channel_->writer_list_;
    auto &buffer = channel_->buffer_;
    // 非空，直接从buffer读
    auto value = buffer.front();
    buffer.pop();
    // 如果有挂起的写者，那么唤醒它
    if (!writer_list.empty()) {
      auto writer = writer_list.front();
      writer_list.pop_front();
      lock.unlock();
      // 调度
      Runtime::GetInstance().getScheduler().SubmitEventRoutine(writer);
    } else {
      lock.unlock();
    }
    return value;
  }
};

template <typename T>
// 实现有缓冲通道
class Channel {
 public:
  friend class WriterAwaiter<T>;
  friend class ReaderAwaiter<T>;
  // 构造函数，设置 Channel 大小，默认大小为 64
  explicit Channel(size_t capacity = channel_size)
      : buffer_capacity_(capacity) {
    // std::cout << "管道" << this << "初始化" << this->buffer_capacity_
    //<< std::endl;
  }
  // 析构函数
  ~Channel() {}
  // 防止 Channel 被移动或者复制
  Channel(Channel &&channel) = delete;
  Channel(Channel &) = delete;
  Channel &operator=(Channel &) = delete;

  // 管道写
  auto write(T value) {
    // std::cout << "管道" << this << "写" << this->buffer_capacity_ <<
    // std::endl;
    return WriterAwaiter<T>(this, value);
  }

  // 管道读
  auto read() {
    // std::cout << "管道" << this << "读" << this->buffer_capacity_ <<
    // std::endl;
    return ReaderAwaiter<T>(this);
  }

 private:
  size_t buffer_capacity_;  // buffer 的容量
  std::queue<T> buffer_;    // 缓冲区
  std::list<std::coroutine_handle<Task::promise_type>>
      writer_list_;  // 写入者挂起队列
  std::list<std::coroutine_handle<Task::promise_type>>
      reader_list_;          // 读取者挂起队列
  std::mutex channel_lock_;  // 锁
};