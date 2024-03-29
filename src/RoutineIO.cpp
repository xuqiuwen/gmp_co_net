#include "./include/RoutineIO.h"

RoutineIO::RoutineIO(size_t loop_size) : event_loop_(loop_size) {}

RoutineReadAwaiter RoutineIO::RoutineRead(int fd, char *buf, size_t nbytes) {
  // 将协程挂起、并调用异步IO
  return RoutineReadAwaiter{fd, buf, nbytes, this};
}

RoutineWriteAwaiter RoutineIO::RoutineWrite(int fd, const char *buf,
                                            size_t nbytes) {
  return RoutineWriteAwaiter{fd, buf, nbytes, this};
}

void RoutineIO::AsyncRead(int fd, char *buf, size_t nbytes) {
  event_loop_.getAsyncIO().async_read(fd, buf, nbytes);
}

void RoutineIO::AsyncWrite(int fd, const char *buf, size_t nbytes) {
  // 不挂起协作，直接异步IO
  event_loop_.getAsyncIO().async_write(fd, buf, nbytes);
}

EventLoop &RoutineIO::getEventLoop() { return event_loop_; }

void RoutineReadAwaiter::await_suspend(
    std::coroutine_handle<Task::promise_type> handle) {
  handle_ = handle;
  routineio_->getEventLoop().EventRegiste(fd_, IOType::Read,
                                          handle);  // 注册事件
  routineio_->AsyncRead(fd_, buf_, nbytes_);        // 异步读
}

void RoutineWriteAwaiter::await_suspend(
    std::coroutine_handle<Task::promise_type> handle) {
  handle_ = handle;
  routineio_->getEventLoop().EventRegiste(fd_, IOType::Write,
                                          handle);  // 注册事件
  routineio_->AsyncWrite(fd_, buf_, nbytes_);       // 异步写
}