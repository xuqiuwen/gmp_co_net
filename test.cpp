#include <iostream>

#include "./src/include/Channel.h"
#include "./src/include/Runtime.h"

#define GO(func) Runtime::GetInstance().getScheduler().SubmitNewRoutine(func)
#define GO_WRITE(a, b, c) \
  co_await Runtime::GetInstance().getRoutineIO().RoutineWrite(a, b, c)
#define GO_READ(a, b, c) \
  co_await Runtime::GetInstance().getRoutineIO().RoutineRead(a, b, c)
#define GOSTART Runtime::GetInstance().Start()
#define GOSTOP Runtime::GetInstance().Stop()

Task sample_coroutine_1() {  // 示例协程函数
  std::cout << "任务1开始输入" << std::endl;
  char buf[100];
  GO_READ(0, buf, 100);  // 调用的异步读
  std::cout << "输入" << buf << "成功" << std::endl;
  char buf1[100] = "hello1\n";
  GO_WRITE(1, buf1, 7);
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
  GO_WRITE(1, buf, 7);
  co_return;
}

Task Sample1() {
  std::cout << "sample1" << std::endl;
  GO_WRITE(1, "hello\n", 7);
  GO_WRITE(1, "bye\n", 5);
  co_return;
}
Task Sample2() {
  std::cout << "sample2" << std::endl;
  GO(Sample1());
  co_return;
}

void routineTest() {
  int n = 100;
  while (n--) {
    GO(Sample2());
  }
}

void IOtest() {
  GO(sample_coroutine_1());
  GO(sample_coroutine_2());
  GO(sample_coroutine_3());
}
Task Producer(Channel<int> &channel) {
  int n = 10;
  while (n--) {
    co_await channel.write(n);
  }
}

Task Consumer(Channel<int> &channel) {
  int n = 10;
  while (n--) {
    auto received = co_await channel.read();
    std::cerr << received << std::endl;
  }
}

void channelTest(Channel<int> &ch) {
  GO(Consumer(ch));
  GO(Producer(ch));
}

int main() {
  GOSTART;
  // 注意channel的生命周期，不能写在ChannelTest里
  IOtest();
  GOSTOP;
}
