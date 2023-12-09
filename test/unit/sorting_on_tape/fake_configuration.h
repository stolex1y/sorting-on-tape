#ifndef FAKE_CONFIGURATION_H
#define FAKE_CONFIGURATION_H

#include "configuration.h"

namespace sot::test {

/**
 * \brief Конфигуратор параметров для тестов.
 */
class FakeConfiguration : public Configuration {
  /**
   * \brief Установить значение задержки чтения.
   */
  template <typename FileTape>
  void SetReadDuration(typename FileTape::Duration duration);
  /**
   * \brief Установить значение задержки записи.
   */
  template <typename FileTape>
  void SetWriteDuration(typename FileTape::Duration duration);
  /**
   * \brief Установить значение задержки сдвига на одну позицию.
   */
  template <typename FileTape>
  void SetMoveDuration(typename FileTape::Duration duration);
  /**
   * \brief Установить значение задержки перемотки.
   */
  template <typename FileTape>
  void SetRewindDuration(typename FileTape::Duration duration);
};

template <typename FileTape>
void FakeConfiguration::SetReadDuration(typename FileTape::Duration duration) {
  params[FileTape::kReadDurationKey] = duration.count();
}

template <typename FileTape>
void FakeConfiguration::SetWriteDuration(typename FileTape::Duration duration) {
  params[FileTape::kWriteDurationKey] = duration.count();
}

template <typename FileTape>
void FakeConfiguration::SetMoveDuration(typename FileTape::Duration duration) {
  params[FileTape::kMoveDurationKey] = duration.count();
}

template <typename FileTape>
void FakeConfiguration::SetRewindDuration(typename FileTape::Duration duration) {
  params[FileTape::kRewingDurationKey] = duration.count();
}

}  // namespace sot::test

#endif  // FAKE_CONFIGURATION_H
