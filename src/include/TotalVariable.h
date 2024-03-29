#pragma once
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>

extern size_t global_queue_size;  // 全局协程队列大小
extern size_t local_queue_size;   // 本地协程队列大小
extern size_t machine_count;      // Machine数量
extern size_t processor_count;    // Processor数量
extern size_t event_size;         // 事件循环大小
extern size_t channel_size;       // 默认channel大小