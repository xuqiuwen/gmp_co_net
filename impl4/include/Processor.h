#pragma once
#include <atomic>
#include <cassert>
#include <optional>

#include "MutexSafeQueue.h"  //模板类不能前置声明吗

class Routine;
class Scheduler;

enum class ProcessorState {
  Idle,     // 队列为空，本地无G
  Running,  // 运行中，本地有G
  Stopped   // 已中止
};

class Processor {
 public:
  Processor(Scheduler *scheduler);
  void Start();
  void Stop();
  bool PushRoutine(Routine routine);
  bool Empty();
  std::optional<Routine> PopRoutine();
  void SetState(ProcessorState state);
  ProcessorState GetState();
  bool GetShutdownFlag();
  void ProcessFunction();

 private:
  MutexSafeQueue<Routine> routines_;
  size_t max_size_;
  std::atomic<bool> shutdown_flag_;
  std::atomic<ProcessorState> state_;
  Scheduler *scheduler_;
};
