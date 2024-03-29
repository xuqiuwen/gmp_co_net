#pragma once
#include <atomic>
#include <cassert>
#include <optional>

#include "./Units/LockFreeQueue.h"
#include "Routine.h"

enum class ProcessorState {
  Idle,     // 队列为空，本地无G
  Running,  // 运行中，本地有G
  Stopped   // 已中止
};

class Processor {
 public:
  Processor();
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
  LockFreeQueue<Routine> routines_;
  size_t max_size_;
  std::atomic<bool> shutdown_flag_;
  std::atomic<ProcessorState> state_;
};
