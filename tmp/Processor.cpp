#include "./include/Processor.h"

#include "./include/Scheduler.h"

void Processor::ProcessFunction() {
  while (true) {
    // debug("Processor的一次循环");
    std::optional<GoRoutine> routine;
    switch (state_.load(std::memory_order_seq_cst)) {
      case ProcessorState::Idle:
        if (!Empty()) {
          state_.store(ProcessorState::Running, std::memory_order_seq_cst);
          // std::cout << "pro_idle->pro_run" << std::endl;
          break;
        }
        // std::cout<< "pro_idle" << std::endl;
        if (shutdown_flag_.load(std::memory_order_seq_cst)) {
          state_.store(ProcessorState::Stopped, std::memory_order_seq_cst);
          // std::cout<< "pro_exit" << std::endl;
          return;
        }
        // 请求帮助
        routine = scheduler_->ScheduleRoutine();
        if (routine.has_value()) {
          routine->resume();
          break;
        }
        break;
      case ProcessorState::Running:
        // std::cout<< "pro_run" << std::endl;
        routine = PopGoroutine();
        if (!routine.has_value()) {
          state_.store(ProcessorState::Idle, std::memory_order_seq_cst);
          // std::cout<< "pro_run->pro_idle" << std::endl;
          break;
        }
        routine->resume();
        break;
      default:
        break;
    }
  }
}
