#pragma once
#include <string>

#include "EventLoop.h"
#include "Scheduler.h"
#include "Task.h"
#include "TotalVariable.h"

class Runtime {  // 支持 C++ 协程的运行时
 public:
  Runtime();
  ~Runtime();
  void GoGo(Task t);

 private:
  Scheduler* s_;
  EventLoop* l_;
  std::jthread event_loop_thread_;
};