#ifndef CALL_CENTER_SRC_CALL_CENTER_TASKS_H_
#define CALL_CENTER_SRC_CALL_CENTER_TASKS_H_

#include <functional>
#include <iostream>

namespace sot::core {

/**
 * \brief Класс-обертка для задачи.
 */
template <typename Task>
class TaskWrapped {
 public:
  explicit TaskWrapped(std::function<Task> task);

  /**
   * \brief Запустить задау.
   */
  void operator()() const;

 private:
  std::function<Task> task_;
};

template <typename Task>
TaskWrapped<Task>::TaskWrapped(std::function<Task> task) : task_(std::move(task)) {
}

template <typename Task>
void TaskWrapped<Task>::operator()() const {
  try {
    task_();
  } catch (const std::exception &ex) {
    std::cerr << "Unhandled std::exception in task: " << ex.what();
  } catch (...) {
    std::cerr << "Unknown unhandled exception in task";
  }
}

}  // namespace sot::core

#endif  // CALL_CENTER_SRC_CALL_CENTER_TASKS_H_
