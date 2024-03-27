#include "./include/Processor.h"

#include "./include/MutexSafeQueue.h"
#include "./include/Routine.h"
#include "./include/Scheduler.h"
#include "./include/TotalVariable.h"

Processor::Processor(Scheduler *scheduler)
    : shutdown_flag_(true),
      state_(ProcessorState::Idle),
      max_size_(local_queue_size),
      scheduler_(scheduler),
      routines_(local_queue_size) {}
void Processor::Start() {
  shutdown_flag_.store(false, std::memory_order_seq_cst);
  state_.store(ProcessorState::Idle, std::memory_order_seq_cst);
}
void Processor::Stop() {
  shutdown_flag_.store(true, std::memory_order_seq_cst);
  state_.store(ProcessorState::Idle, std::memory_order_seq_cst);
}
bool Processor::PushRoutine(Routine routine) { return routines_.Push(routine); }
bool Processor::Empty() { return routines_.Empty(); }
std::optional<Routine> Processor::PopRoutine() { return routines_.Pop(); };
void Processor::SetState(ProcessorState state) {
  state_.store(state, std::memory_order_seq_cst);
}
ProcessorState Processor::GetState() {
  return state_.load(std::memory_order_seq_cst);
}
bool Processor::GetShutdownFlag() {
  return shutdown_flag_.load(std::memory_order_seq_cst);
}

void Processor::ProcessFunction() {
  while (true) {
    // debug("Processor的一次循环");
    std::optional<Routine> routine;
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
        routine = PopRoutine();
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
