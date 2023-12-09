#ifndef TAPE_H
#define TAPE_H

#include <cstdint>
#include <optional>
#include <vector>

namespace sot {

/**
 * \brief Интерфес для работы с устройством типа ленты.
 * \tparam Value тип данных для хранения на устройстве.
 */
template <typename Value = std::int32_t>
class Tape {
 public:
  virtual ~Tape() = default;

  /**
   * \brief Прочитать значение на текущей позиции.
   *
   * \return std::nullopt - если указатель в конце ленты
   */
  [[nodiscard]] virtual std::optional<Value> Read() = 0;
  /**
   * \brief Прочитать указанное количество значений.
   *
   * При этом после выполнения операции указатель находится за последним прочитанном значением.
   *
   * \param n количество значений, которое нужно прочитать.
   * \return список значений, которые были прочитаны, причем количество прочитанных значений может
   * быть меньше чем n, если, например, достигнут конец ленты.
   */
  virtual std::vector<Value> ReadN(size_t n) = 0;
  /**
   * \brief Записать значение на текущую позицию.
   *
   * \return false - если значение не удалось записать.
   */
  virtual bool Write(const Value &value) = 0;
  /**
   * \brief Записать значения, начиная с текущей позиции.
   *
   * При этом после выполнения операции указатель находится за последним записанным значением.
   *
   * \return количество записанных значений.
   */
  virtual size_t WriteN(const std::vector<Value> &values) = 0;
  /**
   * \brief Передвинуть указатель вперед на одну позицию.
   *
   * \return false - если операция не удалась (например, достигнут конец ленты).
   */
  virtual bool MoveForward() = 0;
  /**
   * \brief Передвинуть указатель назад на одну позицию.
   *
   * \return false - если операция не удалась (например, достигнуто начало ленты).
   */
  virtual bool MoveBackward() = 0;
  /**
   * \brief Передвинуть указатель в начало ленты.
   */
  virtual void MoveToBegin() = 0;
  /**
   * \brief Передвинуть указатель в конец ленты.
   *
   * После выполнения операции указатель будет за последним элементом.
   */
  virtual void MoveToEnd() = 0;
};

}  // namespace sot

#endif  // TAPE_H
