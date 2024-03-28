#pragma once
#include "RoutineIO.h"
#include "Scheduler.h"
#include "TotalVariable.h"

class Runtime {
 public:
  void Start();
  void Stop();
  static Runtime& GetInstance();
  Scheduler& getScheduler();

 private:
  Runtime();
  RoutineIO routineio_;
  Scheduler scheduler_;
};