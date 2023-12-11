#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "file_tape.h"

namespace sot::test {
/**
 * \brief Сгенерировать вектор из заданного количества случайных чисел.
 */
std::vector<std::int64_t> GenerateRandomSequence(
    size_t size, std::int64_t seed = std::random_device()()
);

/**
 * \brief Вывести вектор со значениями в поток вывода.
 * \tparam Value значение, преобразуемое в строку через std::to_string.
 */
template <typename Value>
std::ostream &operator<<(std::ostream &out, const std::vector<Value> &values) {
  out << "[";
  for (size_t i = 0; i < values.size(); i++) {
    out << std::to_string(values[i]);
    if (i + 1 < values.size()) {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

/**
 * \brief Прочитать все содержимое устройства.
 *
 * Чтение происходит с заданной позиции.
 * \return прочитанные символы, преобразованные в строку.
 */
std::string ReadAllFromTape(Tape<char> &tape);

/**
 * \brief Прочитать все содержимое устройства.
 *
 * Чтение происходит с заданной позиции.
 */
template <typename Value>
std::vector<Value> ReadAllFromTape(Tape<Value> &tape) {
  return tape.ReadN(SIZE_MAX);
}

/**
 * \brief Замерить время выполнения заданной функции.
 * \tparam MeasureDuration точность измерений.
 * \return продолжительность выполнения функции.
 */
template <typename MeasureDuration>
MeasureDuration Measure(const std::function<void()> &fun) {
  const auto start = std::chrono::steady_clock::now();
  fun();
  const auto end = std::chrono::steady_clock::now();
  return floor<MeasureDuration>(end - start);
}

/**
 * \brief Проверить содержимое вектора на соотвествие заданному вектору.
 */
template <typename Value>
void VerifyContentEquals(const std::vector<Value> &expected, const std::vector<Value> &actual) {
  EXPECT_EQ(expected, actual) << "Actual content is different";
}

/**
 * \brief Проверить содержимое строки на соотвествие заданной строке.
 */
void VerifyContentEquals(const std::string &expected, const std::string &actual);

/**
 * \brief Проверить, что курсор в переданном устройстве находится в конце.
 */
template <typename Value>
void VerifyCursorAtTheEnd(Tape<Value> &tape) {
  EXPECT_EQ(false, tape.MoveForward()) << "Mustn't move forward from the end";
  EXPECT_EQ(std::nullopt, tape.Read()) << "The cursor is not at the end";
}

/**
 * \brief Проверить, что курсор в переданном устройстве находится в начале.
 *
 * Кроме того проверяется равенство первого символа устройства с заданным.
 */
template <typename Value>
void VerifyCursorAtTheBeginning(Tape<Value> &tape, std::optional<Value> first) {
  EXPECT_EQ(false, tape.MoveBackward()) << "Mustn't move backward from the beginning";
  EXPECT_EQ(first, tape.Read()) << "The first value in the tape doesn't match";
}

}  // namespace sot::test

#endif  // TEST_UTILS_H
