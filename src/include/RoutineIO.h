#include <coroutine>

#include "./EventLoop.h"

struct RoutineReadAwaiter;
struct RoutineWriteAwaiter;

class RoutineIO {
 public:
  RoutineIO(size_t loop_size);
  RoutineReadAwaiter RoutineRead(int fd, char *buf, size_t nbytes);
  RoutineWriteAwaiter RoutineWrite(int fd, const char *buf, size_t nbytes);
  void AsyncRead(int fd, char *buf, size_t nbytes);
  void AsyncWrite(int fd, const char *buf, size_t nbytes);
  EventLoop &getEventLoop();

 private:
  EventLoop event_loop_;
};

struct RoutineReadAwaiter {  // 异步读等待器
  int fd_;
  char *buf_;
  size_t nbytes_;
  RoutineIO *routineio_;
  std::coroutine_handle<Task::promise_type> handle_;
  RoutineReadAwaiter(int fd, char *buf, size_t nbytes, RoutineIO *routineio)
      : fd_{fd}, buf_{buf}, nbytes_{nbytes}, routineio_{routineio} {}
  bool await_ready() const noexcept { return false; }  // 上来就挂起
  void await_suspend(std::coroutine_handle<Task::promise_type> handle);
  int await_resume() {
    // std::cerr << "恢复，实际读入" << handle_.promise().getData() <<
    // std::endl;
    return handle_.promise().getData();
  }  // 恢复返回实际读的大小
};

struct RoutineWriteAwaiter {  // 异步写等待器
  int fd_;
  const char *buf_;
  size_t nbytes_;
  RoutineIO *routineio_;
  std::coroutine_handle<Task::promise_type> handle_;
  RoutineWriteAwaiter(int fd, const char *buf, size_t nbytes,
                      RoutineIO *routineio)
      : fd_{fd}, buf_{buf}, nbytes_{nbytes}, routineio_{routineio} {}
  bool await_ready() const noexcept { return false; }  // 上来就挂起
  void await_suspend(std::coroutine_handle<Task::promise_type> handle);
  int await_resume() {
    return handle_.promise().getData();
  }  // 恢复返回实际写的大小
};
