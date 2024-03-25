#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
extern std::atomic<size_t> uncompleted_task_count;  // 统计没完成的任务数目
extern size_t queue_size;                           // 协程队列大小
extern size_t pool_size;                            // 线程池大小
extern size_t loop_size;                            // 事件循环大小
extern size_t channel_size;                         // 默认channel大小
extern std::mutex zero_mtx;                         // 互斥量
extern std::condition_variable zero_cv;             // 条件变量