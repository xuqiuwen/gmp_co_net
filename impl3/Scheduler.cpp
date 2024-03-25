
#include "./include/Scheduler.h"
Scheduler &Scheduler::GetInstance() {
  static Scheduler instance;
  return instance;
}

Scheduler::Scheduler()
    : global_task_queue_(queue_size), worker_pool_(pool_size) {}

Scheduler::~Scheduler() { Shutdown(); }

void Scheduler::Start() {
  worker_pool_.Start();  // 启动线程池
}

void Scheduler::Shutdown() {
  // while (1) {
  //   if (task_queue_.Size() == 0) {
  //     break;
  //   }
  // }
  worker_pool_.Shutdown();
}

void Scheduler::Schedule(const Task &task) {
  uncompleted_task_count.fetch_add(1);
  Schedule(task.handle_);
}

void Scheduler::Schedule(std::coroutine_handle<> &handler) {
  // auto is_ok =
  // local_task_queues_.at(std::this_thread::get_id()).Push(handler);
  // if (!is_ok) {  // 本地队列已满，放入全局队列
  global_task_queue_.Push(handler);
  //}
  // exit(1);  // 报错
}
