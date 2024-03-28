#include "./include/Runtime.h"

void Runtime::Start() {
  routineio_.getEventLoop().Start();
  scheduler_.Start();
}
void Runtime::Stop() {
  scheduler_.Stop();
  routineio_.getEventLoop().Stop();
};

Runtime& Runtime::GetInstance() {
  static Runtime instance;
  return instance;
}

Scheduler& Runtime::getScheduler() { return scheduler_; }

Runtime::Runtime()
    : routineio_(event_size),
      scheduler_(machine_count, processor_count, global_queue_size) {}