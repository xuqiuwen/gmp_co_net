#include "./src/include/Channel.h"
#include "./src/include/Runtime.h"

#define GO(func) Runtime::GetInstance().getScheduler().SubmitNewRoutine(func)
#define GO_WRITE(a, b, c) \
  co_await Runtime::GetInstance().getRoutineIO().RoutineWrite(a, b, c)
#define GO_READ(a, b, c) \
  co_await Runtime::GetInstance().getRoutineIO().RoutineRead(a, b, c)
#define GO_SLEEP(a, b) \
  co_await Runtime::GetInstance().getRoutineIO().RoutineTime(a, b)
#define GO_START LibGoRoutine a
#define GO_CHANNEL Channel

class LibGoRoutine {
 public:
  LibGoRoutine() { Runtime::GetInstance().Start(); }
  ~LibGoRoutine() { Runtime::GetInstance().Stop(); }
};
