#include <atomic>
#include <mutex>
#include <optional>
#include <queue>
#include <unordered_map>

#include "SpinLock.h"

template <typename Key, typename Val>
class SpinLockHashQueue {
 public:
  void push(Key key, Val val);
  std::optional<Val> pop(Key key);

 private:
  std::unordered_map<Key, std::queue<Val>> map_;
  SpinLock mtx_;  // 使用自旋锁代替互斥锁
};

template <typename Key, typename Val>
void SpinLockHashQueue<Key, Val>::push(Key key, Val val) {
  mtx_.lock();  // 加锁
  if (map_.find(key) == map_.end()) {
    map_.insert({key, {}});
  }
  map_[key].push(std::move(val));
  mtx_.unlock();  // 解锁
}

template <typename Key, typename Val>
std::optional<Val> SpinLockHashQueue<Key, Val>::pop(Key key) {
  mtx_.lock();  // 加锁
  if (map_.find(key) == map_.end() || map_[key].empty()) {
    mtx_.unlock();  // 解锁后返回
    return std::nullopt;
  }
  auto ret = std::move(map_[key].front());
  map_[key].pop();
  mtx_.unlock();  // 解锁
  return ret;
}
