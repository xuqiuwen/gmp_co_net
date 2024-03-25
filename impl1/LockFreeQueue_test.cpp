#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "LockFreeQueue.hpp"  // 假设您的无锁队列定义在这个文件中
std::mutex cout_mutex;        // 用于同步访问std::cout的互斥锁

// 生产者函数
void producer(LockFreeQueue<int>& queue, int start, int count,
              std::set<int>& produced) {
  for (int i = start; i < start + count; ++i) {
    while (!queue.push(i)) {
      std::this_thread::yield();  // 如果队列满了，就让出CPU时间片，稍后重试
    }
    produced.insert(i);  // 记录生产的元素
  }
}

// 消费者函数
void consumer(LockFreeQueue<int>& queue, int count, std::set<int>& consumed) {
  int value;
  for (int i = 0; i < count; ++i) {
    while (!queue.pop(value)) {
      std::this_thread::yield();  // 如果队列空了，就让出CPU时间片，稍后重试
    }
    consumed.insert(value);  // 记录消费的元素
  }
}

int main() {
  LockFreeQueue<int> queue(10);
  std::set<int> produced_elements, consumed_elements;
  std::mutex produced_mutex, consumed_mutex;

  const int num_producers = 4;
  const int num_consumers = 4;
  const int items_per_producer = 100000;  // 每个生产者将生产1000个元素

  std::vector<std::thread> threads;

  // 启动生产者线程
  for (int i = 0; i < num_producers; ++i) {
    threads.emplace_back([&]() {
      std::set<int> local_produced;
      producer(queue, i * items_per_producer, items_per_producer,
               local_produced);
      std::lock_guard<std::mutex> lock(produced_mutex);
      produced_elements.insert(local_produced.begin(), local_produced.end());
    });
  }

  // 启动消费者线程
  for (int i = 0; i < num_consumers; ++i) {
    threads.emplace_back([&]() {
      std::set<int> local_consumed;
      consumer(queue, items_per_producer * num_producers / num_consumers,
               local_consumed);
      std::lock_guard<std::mutex> lock(consumed_mutex);
      consumed_elements.insert(local_consumed.begin(), local_consumed.end());
    });
  }

  // 等待所有线程完成
  for (auto& t : threads) {
    t.join();
  }

  // 比较生产和消费的元素
  if (produced_elements == consumed_elements) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "Test passed: All produced elements have been consumed."
              << std::endl;
  } else {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "Test failed: Not all produced elements have been consumed."
              << std::endl;
  }

  return 0;
}
