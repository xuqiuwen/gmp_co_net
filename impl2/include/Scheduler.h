#pragma once
#include <string>

#include "EventLoop.h"
#include "LockFreeQueue.h"
#include "Task.h"
#include "ThreadPool.h"
#include "TotalVariable.h"

class Scheduler {
 public:
  static Scheduler &GetInstance();
  // 指定全局协程队列大小和线程池大小

  // 确保不能复制或移动单例对象
  Scheduler(const Scheduler &) = delete;
  Scheduler &operator=(const Scheduler &) = delete;
  Scheduler(Scheduler &&) = delete;
  Scheduler &operator=(Scheduler &&) = delete;

  // 调度协程
  void Schedule(const Task &task);                  // 第一次调度
  void Schedule(std::coroutine_handle<> &handler);  // 中断后调度

  void Start();     // 启动调度器->启动线程池
  void Shutdown();  // 关闭调度器->关闭线程池

 private:
  Scheduler();  // 私有构造
  ~Scheduler();
  LockFreeQueue<std::coroutine_handle<>> task_queue_;  // 全局协程队列
  ThreadPool thread_pool_;                             // 线程池
  void func();  // 线程池线程的回调函数，用来访问全局协程队列
};