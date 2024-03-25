#include "ThreadPool.hpp"

#include <chrono>
#include <iostream>

void printMessage(int id) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Task " << id << " is running." << std::endl;
}

int calculateSum(int n) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  int sum = 0;
  for (int i = 1; i <= n; ++i) {
    sum += i;
  }
  return sum;
}

int main() {
  // 创建一个拥有4个工作线程的线程池
  ThreadPool pool(4, 100);

  // 提交打印任务
  for (int i = 0; i < 10; ++i) {
    pool.Submit([i]() { printMessage(i); });
  }

  // 提交计算任务
  pool.Submit([]() {
    int sum = calculateSum(10);
    std::cout << "Sum of 1 to 10 is " << sum << std::endl;
  });

  // 给一些时间让线程池处理任务
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // 关闭线程池并等待所有任务完成
  pool.Shutdown();

  return 0;
}
