#include "thread_pool.h"

namespace sot::core {

const size_t ThreadPool::kDefaultMaxThreadCount = std::min(std::thread::hardware_concurrency(), 2u);

ThreadPool::ThreadPool(const size_t max_thread_count) : max_thread_count_(max_thread_count) {
  AddThread();
}

ThreadPool::~ThreadPool() {
  std::lock_guard lock(mutex_);
  done_ = true;
  has_work_.notify_all();
}

void ThreadPool::PostTask(std::function<Task> task) {
  std::lock_guard lock(mutex_);
  work_queue_.emplace(std::move(task));
  if (thread_count_ < max_thread_count_ && free_threads_ == 0) {
    AddThread();
  }
  has_work_.notify_one();
}

bool ThreadPool::HasWorks() const {
  std::lock_guard lock(mutex_);
  return !work_queue_.empty() || thread_count_ != free_threads_;
}

void ThreadPool::AddThread() {
  try {
    threads_.emplace_back([this] {
      ThreadWorker();
    });
    ++thread_count_;
    ++free_threads_;
  } catch (...) {
    done_ = true;
    throw;
  }
}

void ThreadPool::ThreadWorker() {
  while (!done_) {
    std::unique_lock lock(mutex_);
    if (work_queue_.empty()) {
      has_work_.wait(lock, [this] {
        return !work_queue_.empty() || done_;
      });
    }
    if (done_) {
      return;
    }
    const auto task = std::move(work_queue_.front());
    work_queue_.pop();
    --free_threads_;
    lock.unlock();
    task();
    ++free_threads_;
  }
}

}  // namespace sot::core
