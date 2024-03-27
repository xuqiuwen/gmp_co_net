#include "./include/Scheduler.h"
size_t Random(size_t l, size_t r) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(l, r);
  return distrib(gen);
}