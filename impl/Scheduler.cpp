#include <string>

#include "EventLoop.hpp"
#include "LockFreeQueue.hpp"
#include "Routine.hpp"
#include "ThreadPool.hpp"

EventLoop *event_loop;  // 事件循环

Task sample_coroutine_1() {  // 示例协程函数
  // auto h = co_await HandleAwaiterGet{};
  std::cout << "任务2开始I/O" << std::endl;
  char buf[100];
  co_await read_async(0, buf, 100, event_loop);
  std::cout << "输入为" << buf << std::endl;
  co_return;
}

Task sample_coroutine_2() {  // 示例协程函数
  // auto h co_await HandleAwaiterGet{};
  std::cout << "任务2开始计算" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "任务2计算结果: 42" << std::endl;
  co_return;
}

class Scheduler {
 public:
  // 指定全局协程队列大小和线程池大小
  Scheduler(size_t queue_size, size_t pool_size, size_t loop_size);
  // 调度协程
  void Schedule(const Task &task);
  virtual ~Scheduler();
  void Start();     // 启动调度器->启动线程池
  void Shutdown();  // 关闭调度器->关闭线程池

 private:
  LockFreeQueue<std::coroutine_handle<>> task_queue_;  // 全局协程队列
  ThreadPool thread_pool_;                             // 线程池
  EventLoop event_loop_;                               // 事件循环
  void func();  // 线程池线程的回调函数，用来访问全局协程队列
};

Scheduler::Scheduler(size_t queue_size, size_t pool_size, size_t loop_size)
    : task_queue_(queue_size),
      thread_pool_(pool_size, std::bind(&Scheduler::func, this)),
      event_loop_(loop_size) {
  event_loop = &event_loop_;
}

Scheduler::~Scheduler() {}

// 线程执行的函数，从协程队列取出协程并执行
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
  while (1) {
    if (task_queue_.Size() == 0) {
      break;
    }
  }
  thread_pool_.Shutdown();
}

void Scheduler::Schedule(const Task &task) { task_queue_.Push(task.handle_); }

int main() {
  Scheduler s(100, 4, 100);
  s.Start();

  s.Schedule(sample_coroutine_1());
  s.Schedule(sample_coroutine_2());

  event_loop->WaitResume();

  //  std::this_thread::sleep_for(std::chrono::seconds(2));
  s.Shutdown();
}
