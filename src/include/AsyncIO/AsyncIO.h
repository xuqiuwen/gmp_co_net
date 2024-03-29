#pragma once
#include <cstddef>
#include <optional>

enum class IOType : int { Read, Write };

class AsyncIO {
 public:
  // 纯虚函数，派生类需要提供具体实现
  virtual void async_read(int fd, char* buf, size_t count) = 0;
  virtual void async_write(int fd, const char* buf, size_t count) = 0;
  virtual std::optional<std::pair<int, IOType>> wait_for_completion(
      int& nbytes) = 0;

  // 虚析构函数确保派生类的析构函数被正确调用
  virtual ~AsyncIO() {}
};
