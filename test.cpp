

#include <iostream>

#include "./impl4/include/AsyncIO/IOUringAsyncIO.h"

int main() {
  // 示例使用
  IOUringAsyncIO io_uring(8);  // 初始化io_uring实例，队列深度为8

  // 打开文件进行异步读写
  int fd = open("test.txt", O_RDWR | O_CREAT, 0644);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  // 准备读写缓冲区
  const char write_buf[] = "Hello, io_uring!";
  char read_buf[sizeof(write_buf)] = {0};

  // 异步写入
  io_uring.async_write(fd, write_buf, sizeof(write_buf));

  // 等待写完成
  io_uring.wait_for_completion();

  // 异步读取
  io_uring.async_read(fd, read_buf, sizeof(read_buf));

  // 等待读完成
  io_uring.wait_for_completion();

  std::cout << "Async read data: " << read_buf << std::endl;

  close(fd);
  return 0;
}
