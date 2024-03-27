#include <iostream>

#include "./include/Machine.h"
#include "./include/Processor.h"
#include "./include/Routine.h"
#include "./include/Scheduler.h"
#define Go(func) r.GoGo(func)

// void func1() {
//   Routine g(Sample());
//   Processor p(nullptr);
//   p.PushRoutine(g);

//   Machine m(&p);
//   m.Start();
//   p.Start();
//   p.SetState(ProcessorState::Running);
//   m.SetState(MachineState::Executing);
//   std::this_thread::sleep_for(std::chrono::seconds(2));
//   m.Stop();
//   p.Stop();
// }

// void func2() {
//   Routine g(Sample());
//   Scheduler s(machine_count, processor_count);

//   s.Start();
//   s.SubmitRoutine(g);
//   // std::this_thread::sleep_for(std::chrono::seconds(1));
//   s.Stop();
// }
Scheduler& s = Scheduler::GetInstance();

Task Sample1() {
  std::cout << "hello" << std::endl;
  co_return;
}
Task Sample2() {
  std::cout << "hello" << std::endl;
  s.SubmitRoutine(Routine{Sample1()});
  co_return;
}

int main() {
  s.Start();
  int n = 100;
  while (n--) {
    s.SubmitRoutine(Routine{Sample2()});
  }
  // P关闭时只考虑本地队列，放在全局队列要等一等
  // std::this_thread::sleep_for(std::chrono::seconds(1));
  s.Stop();
}
