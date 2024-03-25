#pragma once
#include "Channel.h"

template <typename ValueType>
struct WriterAwaiter {
  Channel<ValueType> *channel;

  // 写入 Channel 的值
  ValueType _value;
  std::coroutine_handle<> handle;

  WriterAwaiter(Channel<ValueType> *channel, ValueType value)
      : channel(channel), _value(value) {}

  bool await_ready() { return false; }

  auto await_suspend(std::coroutine_handle<> coroutine_handle) {
    // 记录协程 handle，恢复时用
    this->handle = coroutine_handle;
    // 将自身传给 Channel，Channel 内部会根据自身状态处理是否立即恢复或者挂起
    channel->try_push_writer(this);
  }

  void await_resume() {
    // Channel 关闭时也会将挂起的读写协程恢复
    // 要检查是否是关闭引起的恢复，如果是，check_closed 会抛出 Channel 关闭异常
    channel->check_closed();
  }

  // Channel 当中恢复该协程时调用 resume 函数
  void resume() {
    // 调度执行
    handle.resume();
  }
};

template <typename ValueType>
struct ReaderAwaiter {
  Channel<ValueType> *channel;
  ValueType _value;
  // 用于 channel >> received; 这种情况
  // 需要将变量的地址传入，协程恢复时写入变量内存
  ValueType *p_value = nullptr;
  std::coroutine_handle<> handle;

  explicit ReaderAwaiter(Channel<ValueType> *channel) : channel(channel) {}

  bool await_ready() { return false; }

  auto await_suspend(std::coroutine_handle<> coroutine_handle) {
    this->handle = coroutine_handle;
    // 将自身传给 Channel，Channel 内部会根据自身状态处理是否立即恢复或者挂起
    channel->try_push_reader(this);
  }

  int await_resume() {
    // Channel 关闭时也会将挂起的读写协程恢复
    // 要检查是否是关闭引起的恢复，如果是，check_closed 会抛出 Channel 关闭异常
    channel->check_closed();
    return _value;
  }

  // Channel 当中正常恢复读协程时调用 resume 函数
  void resume(ValueType value) {
    this->_value = value;
    if (p_value) {
      *p_value = value;
    }
    resume();
  }

  // Channel 关闭时调用 resume() 函数来恢复该协程
  void resume() { handle.resume(); }
};
