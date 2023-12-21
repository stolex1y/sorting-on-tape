#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

#include "tasks.h"

namespace sot::core {

class ThreadPool {
 public:
  using Task = void();

  static const size_t kDefaultMaxThreadCount;

  explicit ThreadPool(size_t max_thread_count = kDefaultMaxThreadCount);
  ~ThreadPool();

  void PostTask(std::function<Task> task);
  bool HasWorks() const;

 private:
  std::atomic_bool done_ = false;
  std::queue<TaskWrapped<Task>> work_queue_;
  std::vector<std::jthread> threads_;
  std::atomic_size_t free_threads_ = 0;
  std::size_t thread_count_ = 0;
  mutable std::mutex mutex_;
  const std::size_t max_thread_count_;
  std::condition_variable has_work_;

  void AddThread();
  void ThreadWorker();
};

}  // namespace sot::core

#endif  // THREAD_POOL_H
