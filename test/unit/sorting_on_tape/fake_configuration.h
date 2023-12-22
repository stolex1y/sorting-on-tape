#ifndef FAKE_CONFIGURATION_H
#define FAKE_CONFIGURATION_H

#include "configuration.h"
#include "file_tape.h"
#include "tape_sorter.h"

namespace sot::test {

using namespace std::chrono_literals;

/**
 * \brief Конфигуратор параметров для тестов.
 */
class FakeConfiguration : public Configuration {
 public:
  /**
   * \brief Установить значение задержки чтения.
   */
  template <typename Duration>
  void SetReadDuration(Duration duration);
  /**
   * \brief Установить значение задержки записи.
   */
  template <typename Duration>
  void SetWriteDuration(Duration duration);
  /**
   * \brief Установить значение задержки сдвига на одну позицию.
   */
  template <typename Duration>
  void SetMoveDuration(Duration duration);
  /**
   * \brief Установить значение задержки перемотки.
   */
  template <typename Duration>
  void SetRewindDuration(Duration duration);
  /**
   * \brief Установить нулевое значение всех задержек.
   */
  void SetZeroDurations();
  /**
   * \brief Установить ограничение по использованию памяти в байтах.
   */
  void SetMemoryLimit(size_t limit_size);
  /**
   * \brief Установить максимальное количество элементов, обрабатываемых одним потоком.
   */
  void SetMaxValueCountPerThread(size_t value_count);
  /**
   * \brief Установить максимальное количество блоков, сливаемых одновременно.
   */
  void SetMaxBlockToMergeCount(size_t count);
};

template <typename Duration>
void FakeConfiguration::SetReadDuration(Duration duration) {
  params[FileTape<>::kReadDurationKey] = std::chrono::floor<FileTape<>::Duration>(duration).count();
}

template <typename Duration>
void FakeConfiguration::SetWriteDuration(Duration duration) {
  params[FileTape<>::kWriteDurationKey] =
      std::chrono::floor<FileTape<>::Duration>(duration).count();
}

template <typename Duration>
void FakeConfiguration::SetMoveDuration(Duration duration) {
  params[FileTape<>::kMoveDurationKey] = std::chrono::floor<FileTape<>::Duration>(duration).count();
}

template <typename Duration>
void FakeConfiguration::SetRewindDuration(Duration duration) {
  params[FileTape<>::kRewindDurationKey] =
      std::chrono::floor<FileTape<>::Duration>(duration).count();
}

inline void FakeConfiguration::SetZeroDurations() {
  SetWriteDuration(0us);
  SetReadDuration(0us);
  SetRewindDuration(0us);
  SetMoveDuration(0us);
}

inline void FakeConfiguration::SetMemoryLimit(const size_t limit_size) {
  params[TapeSorter<FileTape<>::ValueT>::kMemoryLimitKey] = limit_size;
}

inline void FakeConfiguration::SetMaxValueCountPerThread(const size_t value_count) {
  params[TapeSorter<FileTape<>::ValueT>::kMaxValueCountPerThreadKey] = value_count;
}

inline void FakeConfiguration::SetMaxBlockToMergeCount(const size_t count) {
  params[TapeSorter<FileTape<>::ValueT>::kMaxBlockToMergeKey] = count;
}

}  // namespace sot::test

#endif  // FAKE_CONFIGURATION_H
