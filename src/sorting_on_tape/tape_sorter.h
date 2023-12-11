#ifndef TAPE_SORTING_H
#define TAPE_SORTING_H

#include <algorithm>
#include <format>
#include <queue>

#include "configuration.h"
#include "file_tape.h"
#include "memory_literals.h"
#include "tape.h"
#include "temp_tape_provider.h"

namespace sot {

using namespace memory_literals;

/**
 * \brief Класс, реализующий сортировку данных с входной ленты на выходную.
 * \tparam Value тип элементов ленты.
 */
template <typename Value>
class TapeSorter {
 public:
  /// Ключ в конфигурации, задающий лимит использования занимаемой памяти при сортировке в байтах.
  static constexpr auto kMemoryLimitKey = "memory_limit";
  /// Значение лимита использования занимаемой памяти при сортировке по умолчанию.
  static constexpr size_t kDefaultMemoryLimit = 1_GiB;

  TapeSorter(const Configuration &config, std::shared_ptr<TempTapeProvider<Value>> tape_provider);

  /**
   * \brief Выполнить сортировку данных с входной ленты.
   *
   * Результат выводится в выходную ленту.
   *
   * \param[in] input_tape входная лента;
   * \param[out] output_tape выходная лента.
   */
  void Sort(Tape<Value> &input_tape, Tape<Value> &output_tape) const;

 private:
  using TapeSharedPtr = std::shared_ptr<Tape<Value>>;
  using TapeUniquePtr = std::unique_ptr<Tape<Value>>;

  const size_t memory_limit_;
  const std::shared_ptr<TempTapeProvider<Value>> tape_provider_;
  const size_t in_block_count_;

  /**
   * \brief Выполнить слияние двух отсортированных частей массива.
   * \return результат слияния двух частей.
   */
  [[nodiscard]] TapeUniquePtr Merge(
      const TapeUniquePtr &first_tape, const TapeUniquePtr &second_tape
  ) const;
  /**
   * \brief Записать оставшиеся элементы с одного устройства на другое.
   * \param[in] src источник элементов;
   * \param[out] target целевое устройство, куда будут записаны элементы.
   */
  void WriteLeftPart(Tape<Value> &src, Tape<Value> &target) const;
};

template <typename Value>
TapeSorter<Value>::TapeSorter(
    const Configuration &config, std::shared_ptr<TempTapeProvider<Value>> tape_provider
)
    : memory_limit_(config.GetProperty(kMemoryLimitKey, kDefaultMemoryLimit)),
      tape_provider_(std::move(tape_provider)),
      in_block_count_(static_cast<size_t>(static_cast<double>(memory_limit_) / sizeof(Value))) {
  if (in_block_count_ < 4) {
    throw std::invalid_argument(
        std::format("Increase memory limit! Minimum - {} bytes", sizeof(Value) * 4)
    );
  }
}

template <typename Value>
void TapeSorter<Value>::Sort(Tape<Value> &input_tape, Tape<Value> &output_tape) const {
  // прочитать входные данные поблочно, ввиду ограничения использования памяти
  std::queue<TapeUniquePtr> temp_tapes;  // разделенные и отсоритрованные блоки данных
  auto temp_tape_values = input_tape.ReadN(in_block_count_);  // элементы очередного блока
  while (!temp_tape_values.empty()) {
    std::ranges::sort(temp_tape_values);

    auto temp_tape = tape_provider_->Get();  // временное устройство для хранения очередного блока
    temp_tape->WriteN(temp_tape_values);
    temp_tape->MoveToBegin();
    temp_tapes.emplace(std::move(temp_tape));

    temp_tape_values = input_tape.ReadN(in_block_count_);  // прочитать следующий блок данных
  }
  // выполнить попарное слияние блоков данных, пока не останется 1 блок
  while (temp_tapes.size() > 1) {
    const auto temp_tapes_count = temp_tapes.size();
    for (size_t i = 0; i + 1 < temp_tapes_count; i += 2) {
      const auto first_tape = std::move(temp_tapes.front());
      temp_tapes.pop();
      const auto second_tape = std::move(temp_tapes.front());
      temp_tapes.pop();
      temp_tapes.emplace(Merge(first_tape, second_tape));
    }
  }
  if (!temp_tapes.empty()) {
    WriteLeftPart(*temp_tapes.front(), output_tape);
    output_tape.MoveToBegin();
  }
}

template <typename Value>
std::unique_ptr<Tape<Value>> TapeSorter<Value>::Merge(
    const TapeUniquePtr &first_tape, const TapeUniquePtr &second_tape
) const {
  auto merged_tape = tape_provider_->Get();  // устройство для объединенных данных
  // сливание будет выполняться по частям указанного размера
  const auto part_size = in_block_count_ / 4;

  std::vector<Value> first_part = first_tape->ReadN(part_size);
  std::vector<Value> second_part = second_tape->ReadN(part_size);
  std::vector<Value> merged_values;
  merged_values.reserve(first_part.size() + second_part.size());

  size_t first_i = 0, second_i = 0;
  while (first_i < first_part.size() && second_i < second_part.size()) {
    if (first_part[first_i] < second_part[second_i]) {
      merged_values.emplace_back(first_part[first_i]);
      ++first_i;
    } else {
      merged_values.emplace_back(second_part[second_i]);
      ++second_i;
    }
    if (first_i == first_part.size()) {
      // прочитать новую порцию данных из первого устройства
      first_part = first_tape->ReadN(part_size);
      first_i = 0;
    } else if (second_i == second_part.size()) {
      // прочитать новую порцию данных из второго устройства
      second_part = second_tape->ReadN(part_size);
      second_i = 0;
    }
    if (merged_values.size() == part_size * 2) {
      // записать отсортированную часть для освобождения памяти
      merged_tape->WriteN(merged_values);
      merged_values.clear();
    }
  }
  merged_tape->WriteN(merged_values);
  merged_values.clear();

  if (!first_part.empty()) {
    // записать оставшуюся часть из первого устройства
    merged_tape->WriteN(first_part.begin() + first_i, first_part.end());
    first_part.clear();
    WriteLeftPart(*first_tape, *merged_tape);
  } else {
    // записать оставшуюся часть из второго устройства
    merged_tape->WriteN(second_part.begin() + second_i, second_part.end());
    second_part.clear();
    WriteLeftPart(*second_tape, *merged_tape);
  }
  merged_tape->MoveToBegin();
  return std::move(merged_tape);
}

template <typename Value>
void TapeSorter<Value>::WriteLeftPart(Tape<Value> &src, Tape<Value> &target) const {
  auto values = src.ReadN(in_block_count_);
  while (!values.empty()) {
    target.WriteN(values);
    values = src.ReadN(in_block_count_);
  }
}

}  // namespace sot

#endif  // TAPE_SORTING_H
