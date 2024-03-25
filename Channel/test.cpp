#include <coroutine>
#include <deque>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Channel.h"
#include "Task.h"

Task Producer(Channel<int> &channel) {
  int i = 0;
  while (i < 10) {
    co_await channel.write(i++);
  }
}

Task Consumer(Channel<int> &channel) {
  while (1) {
    // 读取时使用 read 函数，表达式的值就是读取的值
    auto received = co_await channel.read();
    std::cerr << received << std::endl;
  }
}

int main() {
  Channel<int> ch(1);
  auto b = Producer(ch);
  b.handle_.resume();
  auto a = Consumer(ch);
  a.handle_.resume();

  return 0;
}