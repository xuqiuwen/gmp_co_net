#include "../include/AsyncIO/IOUringAsyncIO.h"

#include "../include/RoutineIO.h"

IOUringAsyncIO::IOUringAsyncIO(size_t event_count) {
  if (io_uring_queue_init(event_count, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    exit(EXIT_FAILURE);
  }
}
IOUringAsyncIO::~IOUringAsyncIO() { io_uring_queue_exit(&ring); }
void IOUringAsyncIO::async_read(int fd, char* buf, size_t nbytes) {
  std::unique_lock<std::mutex> lock(mtx_);
  struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
  io_uring_prep_read(sqe, fd, buf, nbytes, 0);
  auto data_ptr = new std::pair<int, IOType>(fd, IOType::Read);
  sqe->user_data = reinterpret_cast<unsigned long long>(data_ptr);
  int ret = io_uring_submit(&ring);
  if (ret < 0) {
    perror("io_uring_submit");
    exit(EXIT_FAILURE);
  }
  // std::cout << "提交读操作=" << fd << std::endl;
}
void IOUringAsyncIO::async_write(int fd, const char* buf, size_t nbytes) {
  std::unique_lock<std::mutex> lock(mtx_);
  struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
  io_uring_prep_write(sqe, fd, buf, nbytes, 0);
  auto data_ptr = new std::pair<int, IOType>(fd, IOType::Write);
  sqe->user_data = reinterpret_cast<unsigned long long>(
      data_ptr);  // 要在io_uring_prep_write后
  int ret = io_uring_submit(&ring);
  if (ret < 0) {
    perror("io_uring_submit");
    exit(EXIT_FAILURE);
  }
  // std::cout << "提交写操作fd=" << fd << std::endl;
}

void IOUringAsyncIO::async_time(int time_out_fd, long long second,
                                long long nano_second) {
  __kernel_timespec ts{second, nano_second};
  std::unique_lock<std::mutex> lock(mtx_);
  struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
  // 准备一个定时器事件
  io_uring_prep_timeout(sqe, &ts, 0, 0);
  auto data_ptr = new std::pair<int, IOType>(
      time_out_fd,
      IOType::Timeout);  // 使用 time_out_fd 作为文件描述符标识超时事件
  sqe->user_data = reinterpret_cast<unsigned long long>(data_ptr);
  int ret = io_uring_submit(&ring);
  if (ret < 0) {
    perror("io_uring_submit (timeout)");
    exit(EXIT_FAILURE);
  }
}

std::optional<std::pair<int, IOType>> IOUringAsyncIO::wait_for_completion(
    int& nbytes) {
  struct io_uring_cqe* cqe;
  struct __kernel_timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 100000000;  // 超时时间0.1s
  int ret = io_uring_wait_cqe_timeout(&ring, &cqe, &ts);
  // std::cout << "io_uring_wait_cqe返回一次" << std::endl;
  if (ret == -ETIME) {
    return std::nullopt;
  } else if (ret < 0) {
    perror("wait_for_completion");
    exit(EXIT_FAILURE);
  }
  nbytes = cqe->res;
  // std::cout << "数据大小为" << cqe->res << std::endl;
  auto data_ptr = reinterpret_cast<std::pair<int, IOType>*>(cqe->user_data);
  std::pair<int, IOType> retval = *data_ptr;
  delete data_ptr;  // 记得释放
  // 标记CQE已处理
  io_uring_cqe_seen(&ring, cqe);
  return retval;
}
