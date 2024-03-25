#include "./include/Runtime.h"

Runtime::Runtime()
    : s_(&Scheduler::GetInstance()),              // 调度器
      l_(&EventLoop::GetInstance()),              // 事件循环
      event_loop_thread_([&]() { l_->Run(); }) {  // 启动事件循环(单线程)
  s_->Start();                                    // 启动调度器(线程池)
}

Runtime::~Runtime() {
  std::unique_lock<std::mutex> lock(zero_mtx);
  // std::cout << "获取锁，阻塞" << std::endl;
  zero_cv.wait(lock,
               [] { return uncompleted_task_count == 0; });  // 等待计时器变为0

  // while (1) {  // 忙等结合让出时间片
  //   if (uncompleted_task_count.load(std::memory_order_relaxed) == 0) {
  //     break;
  //   }
  //   std::this_thread::yield();
  // }
  // std::cout << "队列大小" << schedule.task_queue_.Size() << std::endl;
  // std::cout << "开始关机" << std::endl;
  s_->Shutdown();  // 关闭调度器/线程池
  // std::cout << "schedule关机" << std::endl;
  l_->Shutdown();  // 关闭事件循环
  // std::cout << "eventloop关机" << std::endl;
}

void Runtime::GoGo(Task t) { s_->Schedule(t); }