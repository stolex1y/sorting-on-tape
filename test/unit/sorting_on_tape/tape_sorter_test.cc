#include "tape_sorter.h"

#include <gtest/gtest.h>

#include "fake_configuration.h"
#include "file_utils.h"
#include "tape_sorter_test_base.h"
#include "test_utils.h"

namespace sot::test {

using Value = char;

class TapeSorterTest : public TapeSorterTestBase<Value>, public testing::Test {
 public:
  TapeSorterTest();
};

TapeSorterTest::TapeSorterTest() {
  TapeSorterTestBase::SetUp(
      "TapeSorterTest", testing::UnitTest::GetInstance()->current_test_info()->name()
  );
  config_.SetMaxValueCountPerThread(10000);
}

TEST_F(TapeSorterTest, SortRandomArrayAscWithoutMemoryLimit) {
  const auto expected_values = InitInputDataWithRandomValues(100000);

  const auto actual_values = SortTape();

  VerifyContentEquals(expected_values, actual_values);
}

TEST_F(TapeSorterTest, SortRandomArrayDescWithoutMemoryLimit) {
  using Comparator = std::greater<Value>;
  const auto expected_values = InitInputDataWithRandomValues<Comparator>(100000);

  const auto actual_values = SortTape<Comparator>();

  VerifyContentEquals(expected_values, actual_values);
}

TEST_F(TapeSorterTest, SortRandomArrayWithMemoryLimit) {
  constexpr auto value_count = 100000;
  const auto expected_values = InitInputDataWithRandomValues(value_count);
  config_.SetMemoryLimit(value_count * sizeof(Value) / 100);

  const auto actual_values = SortTape();

  VerifyContentEquals(expected_values, actual_values);
}

TEST_F(TapeSorterTest, SortEmptyArray) {
  const std::vector<Value> expected_values = InitInputDataWithRandomValues(0);

  const auto actual_values = SortTape();

  VerifyContentEquals(expected_values, actual_values);
}

TEST_F(TapeSorterTest, SortArrayWithOneElement) {
  const std::vector<Value> expected_values{10};
  CreateFileWithBinaryContent(input_file_path_, expected_values);

  const auto actual_values = SortTape();

  VerifyContentEquals(expected_values, actual_values);
}

TEST_F(TapeSorterTest, SortSortedArray) {
  auto expected_values = GenerateRandomArray<Value>(100000);
  std::ranges::sort(expected_values);
  CreateFileWithBinaryContent(input_file_path_, expected_values);

  const auto actual_values = SortTape();

  VerifyContentEquals(expected_values, actual_values);
}

TEST_F(TapeSorterTest, SortSortedInReverseOrderArray) {
  auto expected_values = GenerateRandomArray<Value>(100000);
  std::ranges::sort(expected_values, std::greater());
  CreateFileWithBinaryContent(input_file_path_, expected_values);
  std::ranges::reverse(expected_values);

  const auto actual_values = SortTape();

  VerifyContentEquals(expected_values, actual_values);
}

TEST_F(TapeSorterTest, TooSmallMemoryLimit) {
  const auto expected_values = InitInputDataWithRandomValues(10);
  config_.SetMemoryLimit(sizeof(Value));

  ASSERT_THROW(SortTape(), std::invalid_argument);
}

}  // namespace sot::test
