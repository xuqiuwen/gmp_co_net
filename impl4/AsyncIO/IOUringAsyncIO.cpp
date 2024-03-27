#include "../include/AsyncIO/IOUringAsyncIO.h"

#include "../include/RoutineIO.h"

IOUringAsyncIO::IOUringAsyncIO() {
  if (io_uring_queue_init(QUEUEDEPTH, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    exit(EXIT_FAILURE);
  }
}
IOUringAsyncIO::~IOUringAsyncIO() { io_uring_queue_exit(&ring); }
void IOUringAsyncIO::async_read(int fd, char* buf, size_t nbytes) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
  sqe->user_data = (unsigned long long)fd;  // 存储fd
  io_uring_prep_read(sqe, fd, buf, nbytes, 0);
  io_uring_submit(&ring);
}
void IOUringAsyncIO::async_write(int fd, const char* buf, size_t nbytes) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
  sqe->user_data = (unsigned long long)fd;
  io_uring_prep_write(sqe, fd, buf, nbytes, 0);
  io_uring_submit(&ring);
}
std::pair<int, IOType> IOUringAsyncIO::wait_for_completion() {
  struct io_uring_cqe* cqe;
  int ret = io_uring_wait_cqe(&ring, &cqe);
  if (ret < 0) {
    perror("wait_for_completion");
    exit(EXIT_FAILURE);
  }
  std::pair<int, IOType> retval;
  retval.first = (int)cqe->user_data;
  if (cqe->flags & IORING_OP_READ) {
    retval.second = IOType::Read;
  } else if (cqe->flags & IORING_OP_WRITE) {
    retval.second = IOType::Read;
  }
  // 标记CQE已处理
  io_uring_cqe_seen(&ring, cqe);
  return retval;
}
