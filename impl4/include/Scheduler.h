#pragma once
#include <cassert>
#include <condition_variable>
#include <memory>
#include <optional>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

#include "MutexSafeQueue.h"  //模板类不能前置声明吗

class Routine;
class Machine;
class Processor;

size_t Random(size_t l, size_t r);

class Scheduler {
 public:
  static Scheduler &GetInstance();
  void Start();
  void Stop();
  void SubmitRoutine(Routine routine);
  // 完成一个协程后，通知调度器
  void CompleteRoutine();

  // 本地没有G了，请求scheduler协助调度，先从全局队列取，没有再偷窃
  std::optional<Routine> ScheduleRoutine();

 private:
  // M-P 映射
  std::vector<size_t> machine_processor_map_;
  std::unordered_map<std::jthread::id, size_t> thread_machine_map_;
  std::vector<std::unique_ptr<Processor>> processors_;
  // 保证先析构machines_，等待线程结束
  std::vector<std::unique_ptr<Machine>> machines_;
  // 全局协程队列
  MutexSafeQueue<Routine> global_queue_routines_;
  std::atomic<size_t> uncompleted_task_count_{0};  // 统计没完成的任务数目
  std::mutex zero_mtx_;                            // 互斥量
  std::condition_variable zero_cv_;  // 条件变量，控制计数器为0退出
                                     // std::atomic<bool> running_flag_;
  Scheduler(size_t machine_num, size_t processor_num, size_t global_queue_size);
  std::optional<Routine> PopRoutine();
  // 获取下标为index的P本地队列的一个routine
  std::optional<Routine> PopRoutineLocal(size_t index);
};
