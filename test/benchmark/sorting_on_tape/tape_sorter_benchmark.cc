#include <benchmark/benchmark.h>

#include "tape_sorter_test_base.h"

namespace sot::test::benchmark {

using Value = std::int64_t;

class TapeSorterBenchmark : public TapeSorterTestBase<Value>, public ::benchmark::Fixture {
 public:
  void SetUp(::benchmark::State &st) override;
  void TearDown(::benchmark::State &st) override;
  void SetUpTapeDurations(std::chrono::milliseconds base_duration = 1ms);
  void SetUpMemoryLimit(std::size_t number_count, uint64_t divider = 8);
};

void TapeSorterBenchmark::SetUp(::benchmark::State &st) {
  Fixture::SetUp(st);
  const auto benchmark_name = st.name();
  const auto test_suit_name = benchmark_name.substr(0, benchmark_name.find('/'));
  const auto test_name = benchmark_name.substr(benchmark_name.find('/') + 1);
  TapeSorterTestBase::SetUp(test_suit_name, test_name);
  SetUpTapeDurations();
}

void TapeSorterBenchmark::TearDown(::benchmark::State &st) {
  Fixture::TearDown(st);
}

void TapeSorterBenchmark::SetUpTapeDurations(const std::chrono::milliseconds base_duration) {
  config_.SetMoveDuration(base_duration / 2);
  config_.SetReadDuration(base_duration);
  config_.SetWriteDuration(base_duration);
  config_.SetRewindDuration(base_duration * 4);
}

void TapeSorterBenchmark::SetUpMemoryLimit(const std::size_t number_count, const uint64_t divider) {
  config_.SetMemoryLimit(number_count * sizeof(Value) / divider);
}

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortRandomArray)(::benchmark::State &state) {
  const auto number_count = state.range(0);
  SetUpMemoryLimit(number_count);
  const auto numbers = InitInputDataWithRandomNumbers(number_count);
  for (auto _ : state) {
    const auto sorted_numbers = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortRandomArray)->Arg(40)->Arg(400)->Arg(4000);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortSortedArray)(::benchmark::State &state) {
  const size_t number_count = state.range(0);
  SetUpMemoryLimit(number_count);
  auto expected_numbers = GenerateRandomArray(number_count);
  std::ranges::sort(expected_numbers);
  CreateFileWithBinaryContent(input_file_path_, expected_numbers);
  for (auto _ : state) {
    const auto sorted_numbers = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortSortedArray)->Arg(40)->Arg(400)->Arg(4000);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortSortedInReverseOrderArray)(::benchmark::State &state) {
  const size_t number_count = state.range(0);
  SetUpMemoryLimit(number_count);
  auto expected_numbers = GenerateRandomArray(number_count);
  std::ranges::sort(expected_numbers, std::greater());
  CreateFileWithBinaryContent(input_file_path_, expected_numbers);
  for (auto _ : state) {
    const auto sorted_numbers = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortSortedInReverseOrderArray)
    ->Arg(40)
    ->Arg(400)
    ->Arg(4000);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortArrayWithMemoryLimit)(::benchmark::State &state) {
  constexpr size_t number_count = 1000;
  SetUpMemoryLimit(number_count, state.range(0));
  const auto numbers = InitInputDataWithRandomNumbers(1000);
  for (auto _ : state) {
    const auto sorted_numbers = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortArrayWithMemoryLimit)->Arg(8)->Arg(4)->Arg(2)->Arg(1);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortArrayWithDifferentDelays)(::benchmark::State &state) {
  constexpr size_t number_count = 500;
  SetUpMemoryLimit(number_count);
  SetUpTapeDurations(state.range(0) * 1ms);
  const auto numbers = InitInputDataWithRandomNumbers(number_count);
  for (auto _ : state) {
    const auto sorted_numbers = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortArrayWithDifferentDelays)->Arg(1)->Arg(4)->Arg(16);

BENCHMARK_MAIN();

}  // namespace sot::test::benchmark
