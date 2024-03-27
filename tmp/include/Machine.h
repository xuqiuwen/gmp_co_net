#pragma once
#include <atomic>
#include <functional>
#include <iostream>
#include <thread>

#include "Processor.h"
#include "io_utils.h"
// 假设已经定义了Processor和Goroutine类

enum class MachineState {
  Spinning,   // 自旋中
  Executing,  // 执行代码中
              // Sleeping,   // 休眠中
  Stopped     // 停止
};

class Machine {
 public:
  Machine()
      : shutdown_flag_(true),
        state_(MachineState::Spinning),
        processor_(nullptr){};
  Machine(Processor* processor)
      : processor_(processor), state_(MachineState::Spinning){};
  void Start() {
    shutdown_flag_.store(false, std::memory_order_seq_cst);
    state_.store(MachineState::Spinning, std::memory_order_seq_cst);
    worker_ = std::jthread([this] { this->ThreadFunction(); });
  }
  void Stop() { shutdown_flag_.store(true, std::memory_order_seq_cst); }
  void SetState(MachineState state) {
    state_.store(state, std::memory_order_seq_cst);
  }
  void SetProcessor(Processor* processor) { processor_ = processor; }
  std::jthread::id GetThreadId() { return worker_.get_id(); }

 private:
  std::atomic<bool> shutdown_flag_;
  std::atomic<MachineState> state_;
  std::jthread worker_;
  Processor* processor_;

  void ThreadFunction() {
    while (true) {
      // debug("Machine的一次循环");
      switch (state_.load(std::memory_order_seq_cst)) {
        case MachineState::Spinning:
          // 有P就转执行态
          if (processor_ != nullptr) {
            // std::cout<< "mac_spin->mac_exe" << std::endl;
            state_.store(MachineState::Executing, std::memory_order_seq_cst);
            break;
          }
          // std::cout<< "mac_spin" << std::endl;
          if (shutdown_flag_.load(std::memory_order_seq_cst)) {
            // std::cout<< "mac_exit" << std::endl;
            return;
          }
        case MachineState::Executing:
          // std::cout<< "mac_exe" << std::endl;
          if (processor_ == nullptr ||
              processor_->GetState() == ProcessorState::Stopped) {
            processor_ = nullptr;  // 释放P
            // std::cout<< "mac_exe->mac_spin" << std::endl;
            state_.store(MachineState::Spinning, std::memory_order_seq_cst);
            break;
          }
          processor_->ProcessFunction();
          break;
        // case MachineState::Sleeping:
        //   break;
        default:
          break;
      }
    }
  }
};