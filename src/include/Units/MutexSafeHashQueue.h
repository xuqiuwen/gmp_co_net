#include <mutex>
#include <optional>
#include <queue>
#include <unordered_map>

template <typename Key, typename Val>
class MutexSafeHashQueue {
 public:
  void push(Key key, Val val);
  std::optional<Val> pop(Key key);

 private:
  std::unordered_map<Key, std::queue<Val>> map_;
  std::mutex mtx_;
};

template <typename Key, typename Val>
void MutexSafeHashQueue<Key, Val>::push(Key key, Val val) {
  std::unique_lock lock(mtx_);
  if (map_.find(key) == map_.end()) {
    // 没找到
    map_.insert({key, {}});
  }
  map_[key].push(val);
}

template <typename Key, typename Val>
std::optional<Val> MutexSafeHashQueue<Key, Val>::pop(Key key) {
  std::unique_lock lock(mtx_);
  if (map_[key].empty()) {
    return std::nullopt;
  }
  auto ret = map_[key].front();
  map_[key].pop();
  return ret;
}