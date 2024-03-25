#pragma once
#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include <exception>
#include <list>
#include <optional>
#include <queue>
#include <string>

#include "ChannelAwaiter.h"

template <typename ValueType>
struct Channel {
  void check_closed() {
    // 如果已经关闭，则退出
    if (!_is_active.load(std::memory_order_relaxed)) {
      exit(1);
    }
  }

  explicit Channel(int capacity = 0) : buffer_capacity(capacity) {
    _is_active.store(true, std::memory_order_relaxed);
  }

  // true 表示 Channel 尚未关闭
  bool is_active() { return _is_active.load(std::memory_order_relaxed); }

  // 关闭 Channel
  void close() {
    bool expect = true;
    // 判断如果已经关闭，则不再重复操作
    // 比较 _is_active 为 true 时才会完成设置操作，并且返回 true
    if (_is_active.compare_exchange_strong(expect, false,
                                           std::memory_order_relaxed)) {
      // 清理资源
      clean_up();
    }
  }

  auto write(ValueType value) {
    check_closed();
    return WriterAwaiter<ValueType>(this, value);
  }

  auto operator<<(ValueType value) { return write(value); }

  ReaderAwaiter<ValueType> read() {
    check_closed();
    return ReaderAwaiter<ValueType>(this);
  }

  auto operator>>(ValueType &value_ref) {
    auto awaiter = read();
    // 保存待赋值的变量的地址，方便后续写入
    awaiter.p_value = &value_ref;
    return awaiter;
  }
  void try_push_writer(WriterAwaiter<ValueType> *writer_awaiter) {
    std::unique_lock lock(channel_lock);
    check_closed();
    // 检查有没有挂起的读取者，对应情况 1
    if (!reader_list.empty()) {
      auto reader = reader_list.front();
      reader_list.pop_front();
      lock.unlock();

      reader->resume(writer_awaiter->_value);
      writer_awaiter->resume();
      return;
    }

    // buffer 未满，对应情况 2
    if (buffer.size() < buffer_capacity) {
      buffer.push(writer_awaiter->_value);
      lock.unlock();
      writer_awaiter->resume();
      return;
    }

    // buffer 已满，对应情况 3
    writer_list.push_back(writer_awaiter);
  }

  void try_push_reader(ReaderAwaiter<ValueType> *reader_awaiter) {
    std::unique_lock lock(channel_lock);
    check_closed();

    // buffer 非空，对应情况 1
    if (!buffer.empty()) {
      auto value = buffer.front();
      buffer.pop();

      if (!writer_list.empty()) {
        // 有挂起的写入者要及时将其写入 buffer 并恢复执行
        auto writer = writer_list.front();
        writer_list.pop_front();
        buffer.push(writer->_value);
        lock.unlock();

        writer->resume();
      } else {
        lock.unlock();
      }

      reader_awaiter->resume(value);
      return;
    }

    // 有写入者挂起，对应情况 2
    if (!writer_list.empty()) {
      auto writer = writer_list.front();
      writer_list.pop_front();
      lock.unlock();

      reader_awaiter->resume(writer->_value);
      writer->resume();
      return;
    }

    // buffer 为空，对应情况 3
    reader_list.push_back(reader_awaiter);
  }

  // 不希望 Channel 被移动或者复制
  Channel(Channel &&channel) = delete;
  Channel(Channel &) = delete;
  Channel &operator=(Channel &) = delete;

  // 销毁时关闭
  ~Channel() { close(); }

 private:
  // buffer 的容量
  int buffer_capacity;
  std::queue<ValueType> buffer;
  // buffer 已满时，新来的写入者需要挂起保存在这里等待恢复
  std::list<WriterAwaiter<ValueType> *> writer_list;
  // buffer 为空时，新来的读取者需要挂起保存在这里等待恢复
  std::list<ReaderAwaiter<ValueType> *> reader_list;
  // Channel 的状态标识
  std::atomic<bool> _is_active;

  std::mutex channel_lock;
  std::condition_variable channel_condition;

  void clean_up() {
    std::lock_guard lock(channel_lock);

    // 需要对已经挂起等待的协程予以恢复执行
    for (auto writer : writer_list) {
      writer->resume();
    }
    writer_list.clear();

    for (auto reader : reader_list) {
      reader->resume();
    }
    reader_list.clear();

    // 清空 buffer
    decltype(buffer) empty_buffer;
    std::swap(buffer, empty_buffer);
  }
};
