#ifndef FAKE_CONFIGURATION_H
#define FAKE_CONFIGURATION_H

#include "configuration.h"
#include "file_tape.h"

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
  SetWriteDuration(0ms);
  SetReadDuration(0ms);
  SetRewindDuration(0ms);
  SetMoveDuration(0ms);
}

}  // namespace sot::test

#endif  // FAKE_CONFIGURATION_H
