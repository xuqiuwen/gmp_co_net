#include <iostream>

#include "./include/AsyncWR.h"
#include "./include/Channel.h"
#include "./include/Runtime.h"
#include "./include/Scheduler.h"
#define Go(func) r.GoGo(func)

Task sample_coroutine_1() {  // 示例协程函数
  std::cout << "任务1开始输入" << std::endl;
  char buf[100];
  co_await read_async(0, buf, 100);  // 调用的异步读
  std::cout << "输入" << buf << "成功" << std::endl;
  char buf1[100] = "hello1\n";
  co_await write_async(1, buf1, 7);
  co_return;
}

Task sample_coroutine_2() {  // 示例协程函数
  std::cout << "任务2开始计算" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "任务2计算结果: 42" << std::endl;
  co_return;
}

Task sample_coroutine_3() {  // 示例协程函数
  std::cout << "任务3打算输出hello" << std::endl;
  char buf[100] = "hello2\n";
  co_await write_async(1, buf, 7);
  co_return;
}

Task Producer(Channel<int> &channel) {
  int i = 0;
  while (i < 10) {
    co_await channel.write(i++);
  }
  channel.close();
}

Task Consumer(Channel<int> &channel) {
  int n = 8;
  while (n--) {
    // 读取时使用 read 函数，表达式的值就是读取的值
    auto received = co_await channel.read();
    std::cerr << received << std::endl;
  }
}

int main() {
  Runtime r;

  Go(sample_coroutine_1());
  Go(sample_coroutine_2());
  Go(sample_coroutine_3());

  Channel<int> ch;
  Go(Consumer(ch));
  Go(Producer(ch));

  return 0;
}
