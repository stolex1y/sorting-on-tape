#ifndef TAPE_PROVIDER_H
#define TAPE_PROVIDER_H

#include <memory>

#include "tape.h"

namespace sot {
/**
 * \brief Класс, предоставляющий устройства типа ленты для временного использования.
 *
 * \warning При уничтожении экзмепляра данного класса, доступ к ленточному устройству теряется.
 *
 * \tparam Value тип элементов ленты.
 */
template <typename Value>
class TempTapeProvider {
 public:
  virtual ~TempTapeProvider() = default;

  /**
   * \brief Предоставляет ленточное устройство.
   */
  [[nodiscard]] virtual std::unique_ptr<Tape<Value>> Get() const = 0;
};

}  // namespace sot

#endif  // TAPE_PROVIDER_H
