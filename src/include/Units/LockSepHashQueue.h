#include <array>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <unordered_map>

template <typename Key, typename Val>
class LockSepHashQueue {
 public:
  void push(Key key, Val val);
  std::optional<Val> pop(Key key);

 private:
  std::unordered_map<Key, std::queue<Val>> map_;
  std::array<std::shared_mutex, 256> locks_;  // 256个读写锁
};

template <typename Key, typename Val>
void LockSepHashQueue<Key, Val>::push(Key key, Val val) {
  auto& lock =
      locks_[std::hash<Key>{}(key) % locks_.size()];  // 使用键的哈希值映射到锁
  std::unique_lock<std::shared_mutex> lk(lock);  // 获取对应的写锁
  if (map_.find(key) == map_.end()) {
    map_.insert({key, {}});
  }
  map_[key].push(std::move(val));
}

template <typename Key, typename Val>
std::optional<Val> LockSepHashQueue<Key, Val>::pop(Key key) {
  auto& lock =
      locks_[std::hash<Key>{}(key) % locks_.size()];  // 使用键的哈希值映射到锁
  std::unique_lock<std::shared_mutex> lk(lock);  // 获取对应的写锁
  auto it = map_.find(key);
  if (it == map_.end() || it->second.empty()) {
    return std::nullopt;
  }
  auto ret = std::move(it->second.front());
  it->second.pop();
  return ret;
}
