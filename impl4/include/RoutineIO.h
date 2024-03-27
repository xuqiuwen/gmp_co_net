#include <queue>
#include <unordered_map>
#include <utility>

#include "./include/AsyncIO/AsyncIO.h"
#include "./include/AsyncIO/IOUringAsyncIO.h"
#include "./include/Routine.h"

enum class IOType : int { Read, Write };

struct RoutineReadAwaiter {  // 异步读等待器
  int fd_;
  char *buf_;
  size_t nbytes_;
  RoutineIO *routineio_;
  RoutineReadAwaiter(int fd, char *buf, size_t nbytes, RoutineIO *routineio)
      : fd_{fd}, buf_{buf}, nbytes_{nbytes}, routineio_{routineio} {}
  bool await_ready() const noexcept { return false; }  // 上来就挂起
  void await_suspend(std::coroutine_handle<> handle) {
    routineio_->EventRegiste(fd_, IOType::Read, handle);  // 注册事件
    routineio_->RoutineRead(fd_, buf_, nbytes_);          // 异步读
  }
  void await_resume() {}  // 恢复什么也不干
};

struct RoutineWriteAwaiter {  // 异步写等待器
  int fd_;
  const char *buf_;
  size_t nbytes_;
  RoutineIO *routineio_;
  RoutineWriteAwaiter(int fd, const char *buf, size_t nbytes,
                      RoutineIO *routineio)
      : fd_{fd}, buf_{buf}, nbytes_{nbytes}, routineio_{routineio} {}
  bool await_ready() const noexcept { return false; }  // 上来就挂起
  void await_suspend(std::coroutine_handle<> handle) {
    routineio_->EventRegiste(fd_, IOType::Write, handle);  // 注册事件
    routineio_->RoutineWrite(fd_, buf_, nbytes_);          // 异步写
  }
  void await_resume() {}  // 恢复什么也不干
};

class RoutineIO {
 public:
  auto RoutineRead(int fd, char *buf, size_t nbytes) {
    // 将协程挂起、并调用异步IO
    return RoutineReadAwaiter{fd, buf, nbytes, this};
  }
  auto RoutineWrite(int fd, const char *buf, size_t nbytes) {
    return RoutineWriteAwaiter{fd, buf, nbytes, this};
  }
  void AsyncWrite(int fd, const char *buf, size_t nbytes) {
    // 不挂起协作，直接异步IO
    asyncio_.async_write(fd, buf, nbytes);
  }
  void AsyncRead(int fd, char *buf, size_t nbytes) {
    asyncio_.async_read(fd, buf, nbytes);
  }

 private:
  // 可以替换为EpollAsyncIO
  IOUringAsyncIO asyncio_;
};
