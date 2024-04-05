#include "../include/AsyncIO/SpinLockIOUringAsyncIO.h"

#include "../include/RoutineIO.h"

SpinLockIOUringAsyncIO::SpinLockIOUringAsyncIO(size_t event_count) {
  if (io_uring_queue_init(event_count, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    exit(EXIT_FAILURE);
  }
}
SpinLockIOUringAsyncIO::~SpinLockIOUringAsyncIO() {
  io_uring_queue_exit(&ring);
}
void SpinLockIOUringAsyncIO::async_read(int fd, char* buf, size_t nbytes) {
  mtx_.lock();
  struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
  io_uring_prep_read(sqe, fd, buf, nbytes, 0);
  auto data_ptr = new std::pair<int, IOType>(fd, IOType::Read);
  sqe->user_data = reinterpret_cast<unsigned long long>(data_ptr);
  int ret = io_uring_submit(&ring);
  if (ret < 0) {
    perror("io_uring_submit");
    exit(EXIT_FAILURE);
  }
  mtx_.unlock();
  // std::cout << "提交读操作=" << fd << std::endl;
}
void SpinLockIOUringAsyncIO::async_write(int fd, const char* buf,
                                         size_t nbytes) {
  mtx_.lock();
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
  mtx_.unlock();
  // std::cout << "提交写操作fd=" << fd << std::endl;
}
std::optional<std::pair<int, IOType>>
SpinLockIOUringAsyncIO::wait_for_completion(int& nbytes) {
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
