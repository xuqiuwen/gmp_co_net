#include "./include/Routine.h"

#include "./include/Runtime.h"
#include "./include/Scheduler.h"

std::suspend_never Task::promise_type::final_suspend() noexcept {
  Runtime::GetInstance().getScheduler().CompleteRoutine();
  return {};
}