#ifndef TAPE_SORTER_TEST_BASE_H
#define TAPE_SORTER_TEST_BASE_H

#include <filesystem>

#include "fake_configuration.h"
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
   * \brief Создать SorterTape и выполнить сортировку.
   */
  [[nodiscard]] std::vector<Value> SortTape() const;
  /**
   * \brief Создать файл, заполненный случайными числами.
   *
   * Файл создатся по пути @link input_file_path_ @endlink.
   *
   * \return сгенерированный массив.
   */
  [[nodiscard]] std::vector<Value> InitInputDataWithRandomNumbers(size_t numbers_count) const;
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
std::vector<Value> TapeSorterTestBase<Value>::SortTape() const {
  FileTape<Value, false> input_tape(config_, input_file_path_);
  FileTape<Value> output_tape(config_, output_file_path_);
  const TapeSorter sorter(config_, tape_provider_);
  sorter.Sort(input_tape, output_tape);
  return ReadAllFromTape(output_tape);
}

template <typename Value>
std::vector<Value> TapeSorterTestBase<Value>::InitInputDataWithRandomNumbers(
    const size_t numbers_count
) const {
  auto expected_numbers = GenerateRandomArray(numbers_count);
  CreateFileWithBinaryContent(input_file_path_, expected_numbers);
  std::ranges::sort(expected_numbers);
  return expected_numbers;
}

}  // namespace sot::test

#endif
