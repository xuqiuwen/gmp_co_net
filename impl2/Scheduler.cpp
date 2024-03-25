
#include "./include/Scheduler.h"
Scheduler &Scheduler::GetInstance() {
  static Scheduler instance;
  return instance;
}

Scheduler::Scheduler()
    : task_queue_(queue_size),
      thread_pool_(pool_size, std::bind(&Scheduler::func, this)) {}

Scheduler::~Scheduler() { Shutdown(); }

// 线程池里线程执行的函数，从协程队列取出协程并执行
void Scheduler::func() {
  std::coroutine_handle<> task;
  bool has = task_queue_.Pop(task);
  if (has) {        // 有任务
    task.resume();  // 唤醒协程并执行
  } else {  // 没任务，针对任务不密集的场景，让出时间片
    std::this_thread::yield();
  }
}

void Scheduler::Start() {
  thread_pool_.Start();  // 启动线程池
}

void Scheduler::Shutdown() {
  // while (1) {
  //   if (task_queue_.Size() == 0) {
  //     break;
  //   }
  // }
  thread_pool_.Shutdown();
}

void Scheduler::Schedule(const Task &task) {
  uncompleted_task_count.fetch_add(1);
  task_queue_.Push(task.handle_);
}

void Scheduler::Schedule(std::coroutine_handle<> &handler) {
  task_queue_.Push(handler);
}
