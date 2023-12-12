#include "tape_sorter.h"

#include <gtest/gtest.h>

#include "fake_configuration.h"
#include "file_utils.h"
#include "tape_sorter_test_base.h"
#include "test_utils.h"

namespace sot::test {

using Value = std::int64_t;

class TapeSorterTest : public TapeSorterTestBase<Value>, public testing::Test {
 public:
  TapeSorterTest();
};

TapeSorterTest::TapeSorterTest() {
  TapeSorterTestBase::SetUp(
      "TapeSorterTest", testing::UnitTest::GetInstance()->current_test_info()->name()
  );
}

TEST_F(TapeSorterTest, SortRandomArrayWithoutMemoryLimit) {
  const auto expected_numbers = InitInputDataWithRandomNumbers(1000000);

  const auto actual_numbers = SortTape();

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortRandomArrayWithMemoryLimit) {
  const auto expected_numbers = InitInputDataWithRandomNumbers(1000);
  config_.SetMemoryLimit(10 * sizeof(Value) + 1);

  const auto actual_numbers = SortTape();

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortEmptyArray) {
  const std::vector<Value> expected_numbers = InitInputDataWithRandomNumbers(0);

  const auto actual_numbers = SortTape();

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortArrayWithOneElement) {
  const std::vector<Value> expected_numbers{10};
  CreateFileWithBinaryContent(input_file_path_, expected_numbers);

  const auto actual_numbers = SortTape();

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortSortedArray) {
  auto expected_numbers = GenerateRandomArray(10000);
  std::ranges::sort(expected_numbers);
  CreateFileWithBinaryContent(input_file_path_, expected_numbers);

  const auto actual_numbers = SortTape();

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, SortSortedInReverseOrderArray) {
  auto expected_numbers = GenerateRandomArray(10000);
  std::ranges::sort(expected_numbers, std::greater());
  CreateFileWithBinaryContent(input_file_path_, expected_numbers);
  std::ranges::reverse(expected_numbers);

  const auto actual_numbers = SortTape();

  VerifyContentEquals(expected_numbers, actual_numbers);
}

TEST_F(TapeSorterTest, TooSmallMemoryLimit) {
  const auto expected_numbers = InitInputDataWithRandomNumbers(10);
  config_.SetMemoryLimit(sizeof(Value));

  ASSERT_THROW(SortTape(), std::invalid_argument);
}

}  // namespace sot::test
