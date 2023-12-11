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
  /// Тип данных для хранения на устройстве.
  using ValueT = Value;
  /// Вектор, состоящий из элементов.
  using Values = std::vector<Value>;
  /// Итератор вектора из элементов.
  using ValuesIterator = typename Values::iterator;
  /// Константный итератор вектора из элементов.
  using ValuesConstIterator = typename Values::const_iterator;

  Tape() = default;
  Tape(const Tape &other) = delete;
  Tape(Tape &&other) = default;
  Tape &operator=(const Tape &other) = delete;
  Tape &operator=(Tape &&other) = default;
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
  virtual Values ReadN(size_t n) = 0;
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
  virtual size_t WriteN(const Values &values) = 0;
  /**
   * \brief Записать значения из заданного диапазона, начиная с текущей позиции.
   *
   * При этом после выполнения операции указатель находится за последним записанным значением.
   *
   * \param begin начало записываемого полуинтервала.
   * \param end конец записываемого полуинтервала.
   * \return итератор, указывающий на последний записанный элемент либо на конец полуинтервала, если
   * были записаны все элементы.
   */
  virtual ValuesConstIterator WriteN(ValuesConstIterator begin, ValuesConstIterator end) = 0;
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
