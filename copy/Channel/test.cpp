#include <coroutine>
#include <deque>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Channel.h"
#include "ChannelAwaiter.h"
#include "Task.h"

struct ChanelTask {
  struct promise_type {
    std::exception_ptr exception;
    std::coroutine_handle<> continuation;

    ChanelTask get_return_object() {
      return ChanelTask{
          std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept {
      if (continuation) continuation.resume();
      return {};
    }
    void unhandled_exception() { exception = std::current_exception(); }
  };

  std::coroutine_handle<promise_type> coro;

  ChanelTask(std::coroutine_handle<promise_type> h) : coro(h) {}
  ~ChanelTask() {
    if (coro) coro.destroy();
  }

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    coro.promise().continuation = h;
  }
};

Task Producer(Channel<int> &channel) {
  int i = 0;
  while (i < 10) {
    // 写入时调用 write 函数
    co_await channel.write(i++);
    // 或者使用 << 运算符
    co_await (channel << i++);
  }

  // 支持关闭
  channel.close();
}

Task Consumer(Channel<int> &channel) {
  while (channel.is_active()) {
    // 读取时使用 read 函数，表达式的值就是读取的值
    // auto received = co_await channel.read();

    int received;
    // 或者使用 >> 运算符将读取的值写入变量当中
    co_await (channel >> received);
    std::cerr << received << std::endl;
  }
}

int main() {
  Channel<int> ch;
  auto a = Consumer(ch);
  a.handle_.resume();
  auto b = Producer(ch);
  b.handle_.resume();
  return 0;
}