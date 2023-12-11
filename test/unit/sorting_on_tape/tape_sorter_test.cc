#include "tape_sorter.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <random>

#include "fake_configuration.h"
#include "file_tape.h"
#include "file_utils.h"
#include "temp_file_tape_provider.h"
#include "test_utils.h"

namespace sot::test {

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::filesystem;
using namespace files;

class TapeSorterTest : public testing::Test {
 public:
  using Value = std::int64_t;

  FakeConfiguration config_;
  std::string test_name_ = testing::UnitTest::GetInstance()->current_test_info()->name();
  path file_prefix_ = path("TapeSorterTest") / test_name_;
  std::shared_ptr<TempTapeProvider<Value>> tape_provider_;

  TapeSorterTest();

  /**
   * \brief Создать SorterTape и выполнить сортировку.
   */
  void SortTape(Tape<Value> &input_tape, Tape<Value> &output_tape) const;
};

TapeSorterTest::TapeSorterTest()
    : tape_provider_(std::make_shared<TempFileTapeProvider<Value>>(config_)) {
  CreateDirWithReplace(file_prefix_);
  config_.SetZeroDurations();
}

void TapeSorterTest::SortTape(Tape<Value> &input_tape, Tape<Value> &output_tape) const {
  const TapeSorter sorter(config_, tape_provider_);
  sorter.Sort(input_tape, output_tape);
}

TEST_F(TapeSorterTest, SortRandomArrayWithoutMemoryLimit) {
  const auto input_file_name = file_prefix_ / "input";
  const auto output_file_name = file_prefix_ / "output";
  constexpr auto numbers_count = 1000;

  auto expected_numbers = GenerateRandomArray(numbers_count);
  CreateFileWithBinaryContent(input_file_name, expected_numbers);

  FileTape<Value, false> input_tape(config_, input_file_name);
  FileTape<Value> output_tape(config_, output_file_name);

  SortTape(input_tape, output_tape);
  const auto actual_numbers = ReadAllFromTape(output_tape);

  std::ranges::sort(expected_numbers);

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortRandomArrayWithMemoryLimit) {
  const auto input_file_name = file_prefix_ / "input";
  const auto output_file_name = file_prefix_ / "output";
  constexpr auto numbers_count = 100000;

  config_.SetMemoryLimit(1_KiB);

  auto expected_numbers = GenerateRandomArray(numbers_count);
  CreateFileWithBinaryContent(input_file_name, expected_numbers);

  FileTape<Value, false> input_tape(config_, input_file_name);
  FileTape<Value> output_tape(config_, output_file_name);

  SortTape(input_tape, output_tape);
  const auto actual_numbers = ReadAllFromTape(output_tape);

  std::ranges::sort(expected_numbers);

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortEmptyArray) {
  const auto input_file_name = file_prefix_ / "input";
  const auto output_file_name = file_prefix_ / "output";

  constexpr std::vector<Value> expected_numbers;
  CreateFileWithBinaryContent(input_file_name, expected_numbers);

  FileTape<Value, false> input_tape(config_, input_file_name);
  FileTape<Value> output_tape(config_, output_file_name);

  SortTape(input_tape, output_tape);
  const auto actual_numbers = ReadAllFromTape(output_tape);

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortArrayWithOneElement) {
  const auto input_file_name = file_prefix_ / "input";
  const auto output_file_name = file_prefix_ / "output";

  const std::vector<Value> expected_numbers{10};
  CreateFileWithBinaryContent(input_file_name, expected_numbers);

  FileTape<Value, false> input_tape(config_, input_file_name);
  FileTape<Value> output_tape(config_, output_file_name);

  SortTape(input_tape, output_tape);
  const auto actual_numbers = ReadAllFromTape(output_tape);

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortSortedArray) {
  const auto input_file_name = file_prefix_ / "input";
  const auto output_file_name = file_prefix_ / "output";
  constexpr auto numbers_count = 10000;

  auto expected_numbers = GenerateRandomArray(numbers_count);
  std::ranges::sort(expected_numbers);
  CreateFileWithBinaryContent(input_file_name, expected_numbers);

  FileTape<Value, false> input_tape(config_, input_file_name);
  FileTape<Value> output_tape(config_, output_file_name);

  SortTape(input_tape, output_tape);
  const auto actual_numbers = ReadAllFromTape(output_tape);

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortSortedInReverseOrderArray) {
  const auto input_file_name = file_prefix_ / "input";
  const auto output_file_name = file_prefix_ / "output";
  constexpr auto numbers_count = 10000;

  auto expected_numbers = GenerateRandomArray(numbers_count);
  std::ranges::sort(expected_numbers, std::greater());
  CreateFileWithBinaryContent(input_file_name, expected_numbers);

  FileTape<Value, false> input_tape(config_, input_file_name);
  FileTape<Value> output_tape(config_, output_file_name);

  SortTape(input_tape, output_tape);
  const auto actual_numbers = ReadAllFromTape(output_tape);

  std::ranges::reverse(expected_numbers);

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, TooSmallMemoryLimit) {
  const auto input_file_name = file_prefix_ / "input";
  const auto output_file_name = file_prefix_ / "output";
  constexpr auto numbers_count = 10;

  config_.SetMemoryLimit(sizeof(Value));
  const auto expected_numbers = GenerateRandomArray(numbers_count);
  CreateFileWithBinaryContent(input_file_name, expected_numbers);

  FileTape<Value, false> input_tape(config_, input_file_name);
  FileTape<Value> output_tape(config_, output_file_name);

  ASSERT_THROW(SortTape(input_tape, output_tape), std::invalid_argument);
}

}  // namespace sot::test
