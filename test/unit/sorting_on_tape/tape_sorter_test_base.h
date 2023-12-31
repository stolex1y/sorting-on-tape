#ifndef TAPE_SORTER_TEST_BASE_H
#define TAPE_SORTER_TEST_BASE_H

#include <filesystem>

#include "file_tape.h"
#include "file_utils.h"
#include "tape_sorter.h"
#include "temp_file_tape_provider.h"
#include "test_base.h"
#include "test_utils.h"

namespace sot::test {

using std::filesystem::path;
using namespace files;

/**
 * \brief Базовый класс для тестов класса @link TapeSorter @endlink.
 * \tparam Value тип данных, хранимый на лентах.
 */
template <typename Value>
class TapeSorterTestBase : public TestBase {
 public:
  std::shared_ptr<TempTapeProvider<Value>> tape_provider_;
  path input_file_path_;
  path output_file_path_;

  void SetUp(const std::string& test_suit_name, const std::string& test_name) override;
  /**
   * \brief Создать SorterTape и выполнить сортировку с использованием переданного компаратора.
   *
   * \tparam Comparator компаратор, по умолчанию используется std::less<Value>.
   */
  template <typename Comparator = std::less<Value>>
  [[nodiscard]] std::vector<Value> SortTape() const;
  /**
   * \brief Создать файл, заполненный случайными значениями.
   *
   * Файл создатся по пути @link input_file_path_ @endlink.
   *
   * \tparam Comparator компаратор, по умолчанию используется std::less<Value>.
   * \return сгенерированный массив, отсортированный с использованием переданного компаратора.
   */
  template <typename Comparator = std::less<Value>>
  [[nodiscard]] std::vector<Value> InitInputDataWithRandomValues(size_t values_count) const;
};

template <typename Value>
void TapeSorterTestBase<Value>::SetUp(
    const std::string& test_suit_name, const std::string& test_name
) {
  TestBase::SetUp(test_suit_name, test_name);
  tape_provider_ = std::make_shared<TempFileTapeProvider<Value>>(config_);
  input_file_path_ = file_prefix_ / "input";
  output_file_path_ = file_prefix_ / "output";
}

template <typename Value>
template <typename Comparator>
std::vector<Value> TapeSorterTestBase<Value>::SortTape() const {
  FileTape<Value, false> input_tape(config_, input_file_path_);
  FileTape<Value> output_tape(config_, output_file_path_);
  const TapeSorter<Value, Comparator> sorter(config_, tape_provider_);
  sorter.Sort(input_tape, output_tape);
  return ReadAllFromTape(output_tape);
}

template <typename Value>
template <typename Comparator>
std::vector<Value> TapeSorterTestBase<Value>::InitInputDataWithRandomValues(
    const size_t values_count
) const {
  auto expected_values = GenerateRandomArray<Value>(values_count);
  CreateFileWithBinaryContent(input_file_path_, expected_values);
  std::ranges::sort(expected_values, Comparator());
  return expected_values;
}

}  // namespace sot::test

#endif
