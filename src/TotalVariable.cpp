#include "./include/TotalVariable.h"

size_t global_queue_size = 128;  // 全局协程队列大小
size_t local_queue_size = 64;    // 本地协程队列大小
size_t machine_count = 1;        // M数量
size_t processor_count = 1;      // P数量
size_t event_size = 256;         // 事件循环大小
size_t channel_size = 64;        // 默认channel大小