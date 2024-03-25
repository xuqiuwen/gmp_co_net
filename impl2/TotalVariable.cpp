#include "./include/TotalVariable.h"

std::atomic<size_t> uncompleted_task_count{0};  // 统计没完成的任务数目
size_t queue_size = 128;                        // 协程队列大小
size_t pool_size = 4;                           // 线程池大小
size_t loop_size = 1024;                        // 事件循环大小
size_t channel_size = 64;                       // 默认channel大小
std::mutex zero_mtx;                            // 互斥量
std::condition_variable zero_cv;  // 条件变量，控制计数器为0退出