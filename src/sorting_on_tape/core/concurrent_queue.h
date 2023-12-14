#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>

namespace sot::core {

template <typename T>
class ConcurrentQueue {
 public:
  virtual ~ConcurrentQueue() = default;

  void Push(T new_value);
  std::optional<T> Pop();
  bool Empty() const;
  size_t Size() const;

 protected:
  mutable std::shared_mutex mutex_;
  std::queue<T> queue_;
};

template <typename T>
void ConcurrentQueue<T>::Push(T new_value) {
  std::lock_guard lock(mutex_);
  queue_.push(std::move(new_value));
}

template <typename T>
std::optional<T> ConcurrentQueue<T>::Pop() {
  std::lock_guard lock(mutex_);
  if (queue_.empty()) {
    return std::nullopt;
  }
  T value(std::move(queue_.front()));
  queue_.pop();
  return value;
}

template <typename T>
bool ConcurrentQueue<T>::Empty() const {
  std::shared_lock lock(mutex_);
  return queue_.empty();
}

template <typename T>
size_t ConcurrentQueue<T>::Size() const {
  std::shared_lock lock(mutex_);
  return queue_.size();
}

}  // namespace sot::core

#endif  // CONCURRENT_QUEUE_H
