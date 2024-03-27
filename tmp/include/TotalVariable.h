#pragma once
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>

extern std::atomic<size_t> uncompleted_task_count;  // 统计没完成的任务数目
extern size_t global_queue_size;                    // 全局协程队列大小
extern size_t local_queue_size;                     // 本地协程队列大小
extern size_t machine_count;                        // Machine数量
extern size_t processor_count;                      // Processor数量
extern size_t loop_size;                            // 事件循环大小
extern size_t channel_size;                         // 默认channel大小
// extern std::mutex zero_mtx;                         // 互斥量
// extern std::condition_variable zero_cv;             // 条件变量