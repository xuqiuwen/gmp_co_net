#pragma once
#include <atomic>
#include <functional>
#include <iostream>
#include <thread>

class Processor;

enum class MachineState {
  Spinning,   // 自旋中
  Executing,  // 执行代码中
              // Sleeping,   // 休眠中
  Stopped     // 停止
};

class Machine {
 public:
  Machine();
  Machine(Processor* processor);
  void Start();
  void Stop();
  void SetState(MachineState state);
  void SetProcessor(Processor* processor);
  std::jthread::id GetThreadId();

 private:
  std::atomic<bool> shutdown_flag_;
  std::atomic<MachineState> state_;
  std::jthread worker_;
  Processor* processor_;

  void ThreadFunction();
};