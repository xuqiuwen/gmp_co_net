#include "./include/Routine.h"

#include "./include/Scheduler.h"

std::suspend_never Task::promise_type::final_suspend() noexcept {
  Scheduler::GetInstance().CompleteRoutine();
  return {};
}