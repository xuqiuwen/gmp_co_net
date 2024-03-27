#pragma once
#include <cassert>
#include <optional>

#include "GoRoutine.h"
#include "MutexSafeQueue.h"

class Scheduler;

enum class ProcessorState {
  Idle,     // 队列为空，本地无G
  Running,  // 运行中，本地有G
  Stopped   // 已中止
};

class Processor {
 public:
  Processor(Scheduler *scheduler)
      : shutdown_flag_(true),
        state_(ProcessorState::Idle),
        max_size_(local_queue_size),
        scheduler_(scheduler),
        routines_(local_queue_size){};
  void Start() {
    shutdown_flag_.store(false, std::memory_order_seq_cst);
    state_.store(ProcessorState::Idle, std::memory_order_seq_cst);
  };
  void Stop() {
    shutdown_flag_.store(true, std::memory_order_seq_cst);
    state_.store(ProcessorState::Idle, std::memory_order_seq_cst);
  };
  bool PushGoroutine(GoRoutine routine) { return routines_.Push(routine); };
  bool Empty() { return routines_.Empty(); }
  std::optional<GoRoutine> PopGoroutine() { return routines_.Pop(); };
  void SetState(ProcessorState state) {
    state_.store(state, std::memory_order_seq_cst);
  }
  ProcessorState GetState() { return state_.load(std::memory_order_seq_cst); }
  bool GetShutdownFlag() {
    return shutdown_flag_.load(std::memory_order_seq_cst);
  }
  void ProcessFunction();

 private:
  MutexSafeQueue<GoRoutine> routines_;
  size_t max_size_;
  std::atomic<bool> shutdown_flag_;
  std::atomic<ProcessorState> state_;
  Scheduler *scheduler_;
};
