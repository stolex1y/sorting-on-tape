#ifndef FILE_TAPE_IMPL_H
#define FILE_TAPE_IMPL_H

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#include "configuration.h"
#include "tape.h"

namespace sot {

/**
 * \brief Эмулятор устройства типа ленты на основе файла.
 *
 * \tparam Value тип данных для хранения на устройстве.
 * \tparam Mutable можно ли изменять данные в файле.
 */
template <typename Value = int32_t, bool Mutable = true>
class FileTape : public Tape<Value> {
 public:
  /// Тип данных для хранения на устройстве.
  using ValueT = typename Tape<Value>::ValueT;
  /// Вектор, состоящий из элементов.
  using Values = typename Tape<Value>::Values;
  /// Итератор вектора из элементов.
  using ValuesIterator = typename Tape<Value>::ValuesIterator;
  /// Константный итератор вектора из элементов.
  using ValuesConstIterator = typename Tape<Value>::ValuesConstIterator;
  /// Единицы измерения "задержек" устройства.
  using Duration = std::chrono::microseconds;

  /// Ключ в конфигурации,
  /// задающий задержку чтения с устройства в @link Duration заданных единицах@endlink.
  static constexpr auto kReadDurationKey = "read_duration";
  /// Ключ в конфигурации,
  /// задающий задержку записи на устройства в @link Duration заданных единицах@endlink.
  static constexpr auto kWriteDurationKey = "write_duration";
  /// Ключ в конфигурации,
  /// задающий задержку перемотки ленты в @link Duration заданных единицах@endlink.
  static constexpr auto kMoveDurationKey = "move_duration";
  /// Ключ в конфигурации,
  /// задающий задержку сдвига ленты на одну позицию в @link Duration заданных единицах@endlink.
  static constexpr auto kRewindDurationKey = "rewind_duration";

  /// Значение задержки чтения по умолчанию в @link Duration заданных единицах@endlink.
  static constexpr auto kReadDurationDefault = 7;
  /// Значение задержки записи по умолчанию в @link Duration заданных единицах@endlink.
  static constexpr auto kWriteDurationDefault = 7;
  /// Значение задержки перемотки по умолчанию в @link Duration заданных единицах@endlink.
  static constexpr auto kMoveDurationDefault = 1;
  /// Значение задержки сдвига по умолчанию в @link Duration заданных единицах@endlink.
  static constexpr auto kRewindDurationDefault = 100;

  FileTape(const Configuration &config, const std::string &file_name);
  FileTape(FileTape &&other) = default;
  FileTape(const FileTape &other) = delete;
  FileTape &operator=(const FileTape &other) = delete;
  FileTape &operator=(FileTape &&other) = default;

  std::optional<Value> Read() override;
  Values ReadN(size_t n) override;
  bool Write(const Value &value) override;
  size_t WriteN(const Values &values) override;
  ValuesConstIterator WriteN(ValuesConstIterator begin, ValuesConstIterator end) override;
  bool MoveForward() override;
  bool MoveBackward() override;
  void MoveToBegin() override;
  void MoveToEnd() override;

 private:
  mutable std::fstream fstream_;
  Duration read_duration_;
  Duration write_duration_;
  Duration move_duration_;
  Duration rewind_duration_;

  /**
   * \brief Получить последнюю позицию в файле.
   */
  std::fstream::pos_type GetLastPos() const;
};

template <typename Value, bool Mutable>
FileTape<Value, Mutable>::FileTape(const Configuration &config, const std::string &file_name)
    : read_duration_(config.GetProperty(kReadDurationKey, kReadDurationDefault)),
      write_duration_(config.GetProperty(kWriteDurationKey, kWriteDurationDefault)),
      move_duration_(config.GetProperty(kMoveDurationKey, kMoveDurationDefault)),
      rewind_duration_(config.GetProperty(kRewindDurationKey, kRewindDurationDefault)) {
  auto mode = std::ios_base::in | std::ios_base::binary;
  if (Mutable) {
    mode |= std::ios_base::out;
  }
  if (!std::filesystem::exists(file_name)) {
    if (Mutable) {
      mode |= std::ios_base::trunc;
    }
  }
  fstream_ = std::fstream(file_name, mode);
  if (!fstream_) {
    throw std::invalid_argument("Couldn't open the file with name '" + file_name + "'.");
  }
}

template <typename Value, bool Mutable>
std::optional<Value> FileTape<Value, Mutable>::Read() {
  Value value;
  const auto before = fstream_.tellg();
  fstream_.read(reinterpret_cast<char *>(&value), sizeof(value));
  if (!fstream_) {
    fstream_.clear();
    fstream_.seekg(before);
    return std::nullopt;
  }
  std::this_thread::sleep_for(read_duration_ + move_duration_);
  return value;
}

template <typename Value, bool Mutable>
auto FileTape<Value, Mutable>::ReadN(const size_t n) -> Values {
  std::vector<Value> values;
  for (size_t i = 0; i < n; ++i) {
    const auto value = Read();
    if (!value) {
      break;
    }
    values.push_back(*value);
  }
  return values;
}

template <typename Value, bool Mutable>
bool FileTape<Value, Mutable>::Write(const Value &value) {
  const auto before = fstream_.tellg();
  fstream_.write(reinterpret_cast<const char *>(&value), sizeof(value));
  if (!fstream_) {
    fstream_.clear();
    fstream_.seekg(before);
    return false;
  }
  std::this_thread::sleep_for(write_duration_ + move_duration_);
  return true;
}

template <typename Value, bool Mutable>
size_t FileTape<Value, Mutable>::WriteN(const Values &values) {
  size_t i;
  for (i = 0; i < values.size(); ++i) {
    if (!Write(values[i])) {
      break;
    }
  }
  return i;
}

template <typename Value, bool Mutable>
auto FileTape<Value, Mutable>::WriteN(ValuesConstIterator begin, ValuesConstIterator end)
    -> ValuesConstIterator {
  while (begin < end) {
    if (!Write(*begin)) {
      break;
    }
    ++begin;
  }
  return begin;
}

template <typename Value, bool Mutable>
bool FileTape<Value, Mutable>::MoveForward() {
  const auto before = fstream_.tellg();
  const auto next_pos = fstream_.tellg() + sizeof(Value);
  if (!Mutable) {
    const auto last_pos = GetLastPos();
    if (next_pos > last_pos) {
      return false;
    }
  }
  fstream_.seekg(next_pos);
  if (!fstream_) {
    fstream_.clear();
    fstream_.seekg(before);
    return false;
  }
  std::this_thread::sleep_for(move_duration_);
  return true;
}

template <typename Value, bool Mutable>
bool FileTape<Value, Mutable>::MoveBackward() {
  const auto before = fstream_.tellg();
  fstream_.seekg(-sizeof(Value), std::ios_base::cur);
  if (!fstream_) {
    fstream_.clear();
    fstream_.seekg(before);
    return false;
  }
  std::this_thread::sleep_for(move_duration_);
  return true;
}

template <typename Value, bool Mutable>
void FileTape<Value, Mutable>::MoveToBegin() {
  std::this_thread::sleep_for(rewind_duration_);
  fstream_.seekg(0);
}

template <typename Value, bool Mutable>
void FileTape<Value, Mutable>::MoveToEnd() {
  std::this_thread::sleep_for(rewind_duration_);
  fstream_.seekg(0, std::ios_base::end);
}

template <typename Value, bool Mutable>
std::fstream::pos_type FileTape<Value, Mutable>::GetLastPos() const {
  const auto prev_pos = fstream_.tellg();
  fstream_.seekg(0, std::ios_base::end);
  const auto last_pos = fstream_.tellg();
  fstream_.seekg(prev_pos);
  return last_pos;
}

}  // namespace sot

#endif  // FILE_TAPE_IMPL_H
