#pragma once
#include <cassert>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

#include "GoRoutine.h"
#include "Machine.h"
#include "Processor.h"

size_t Random(size_t l, size_t r);

class Scheduler {
 public:
  static Scheduler &GetInstance() {
    static Scheduler instance(machine_count, processor_count,
                              global_queue_size);
    return instance;
  }
  ~Scheduler(){};
  void Start() {
    // running_flag_.store(true, std::memory_order_seq_cst);
    for (auto &processor : processors_) {
      processor->Start();
    }
    // 目前设计为 M-P 为一对一映射
    for (size_t i = 0; i < machines_.size(); i++) {
      machines_[i]->SetProcessor(processors_[i].get());
      machines_[i]->Start();
      thread_machine_map_.emplace(machines_[i]->GetThreadId(), i);
    };
  }
  void Stop() {
    std::unique_lock<std::mutex> lock(zero_mtx_);
    zero_cv_.wait(lock, [&] { return uncompleted_task_count_ == 0; });
    // running_flag_.store(false, std::memory_order_seq_cst);
    for (auto &processor : processors_) {
      processor->Stop();
    }
    for (auto &machine : machines_) {
      machine->Stop();
    };
  }
  // bool SubmitRoutineGlobal(GoRoutine routine) {  // 放入全局队列
  //   return global_queue_routines_.Push(routine);
  // }
  // 先放入本地，满了就放入全局
  void SubmitRoutine(GoRoutine routine) {
    uncompleted_task_count_.fetch_add(1);
    // 没找到P，主线程就找不到，直接放入全局队列
    if (thread_machine_map_.find(std::this_thread::get_id()) ==
        thread_machine_map_.end()) {
      global_queue_routines_.Push(routine);
      return;
    }
    // 获取线程id对应的machine
    size_t machine_id = thread_machine_map_[std::this_thread::get_id()];
    // 执行存放策略，先放本地队列，放不下再放入全局队列
    size_t processor_id = machine_processor_map_[machine_id];
    auto &processor = *(processors_[processor_id]);
    if (processor.PushGoroutine(routine)) {
      return;
    }
    if (global_queue_routines_.Push(routine)) {
      return;
    }
    exit(1);
  };
  // 完成一个协程后，通知调度器
  void CompleteRoutine() {
    uncompleted_task_count_.fetch_sub(1);
    if (uncompleted_task_count_ == 0) {
      zero_cv_.notify_one();
    }
  }

  // 本地没有G了，请求scheduler协助调度，先从全局队列取，没有再偷窃
  std::optional<GoRoutine> ScheduleRoutine() {
    // debug("请求Scheduler协助");
    auto routine = PopGoroutine();
    if (routine.has_value()) {
      return routine;
    }
    // 随机选取一个本地队列窃取
    return PopGoroutineLocal(Random(0, processor_count - 1));
  };

 private:
  // M-P 映射
  std::vector<size_t> machine_processor_map_;
  std::unordered_map<std::jthread::id, size_t> thread_machine_map_;
  std::vector<std::unique_ptr<Processor>> processors_;
  // 保证先析构machines_，等待线程结束
  std::vector<std::unique_ptr<Machine>> machines_;
  // 全局协程队列
  MutexSafeQueue<GoRoutine> global_queue_routines_;
  std::atomic<size_t> uncompleted_task_count_{0};  // 统计没完成的任务数目
  std::mutex zero_mtx_;                            // 互斥量
  std::condition_variable zero_cv_;  // 条件变量，控制计数器为0退出
                                     // std::atomic<bool> running_flag_;
  Scheduler(size_t machine_num, size_t processor_num, size_t global_queue_size)
      : machine_processor_map_(machine_num),
        machines_(machine_num),
        processors_(processor_num),
        // running_flag_(false),
        global_queue_routines_(global_queue_size) {
    for (size_t i = 0; i < machine_num; i++) {
      machines_[i] = std::make_unique<Machine>();
      // thread_machine_map_.emplace(machines_[i]->,i);
    }
    for (size_t i = 0; i < processor_num; i++) {
      processors_[i] = std::make_unique<Processor>(this);
    }
    for (size_t i = 0; i < machines_.size(); i++) {
      machine_processor_map_[i] = i;
    }
  }  // 如果是无锁队列，记得设置大小
  // 有则返回，无则返回nullopt，要是原子的，因此上面的不行
  std::optional<GoRoutine> PopGoroutine() {
    // debug("全局队列pop");
    return global_queue_routines_.Pop();
  }
  // 获取下标为index的P本地队列的一个routine
  std::optional<GoRoutine> PopGoroutineLocal(size_t index) {
    // debug("窃取");
    return processors_[index]->PopGoroutine();
  }
};
