#include <benchmark/benchmark.h>

#include "tape_sorter_test_base.h"

namespace sot::test::benchmark {

using Value = std::int32_t;

class TapeSorterBenchmark : public TapeSorterTestBase<Value>, public ::benchmark::Fixture {
 public:
  void SetUp(::benchmark::State &st) override;
  void TearDown(::benchmark::State &st) override;
  void SetUpTapeDurations(std::chrono::microseconds base_duration = 5us);
  void SetUpMemoryLimit(std::size_t value_count, uint64_t divider = 10);
  void SetUpPerThreadCount(std::size_t value_count);
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

void TapeSorterBenchmark::SetUpTapeDurations(const std::chrono::microseconds base_duration) {
  config_.SetMoveDuration(base_duration / 5);
  config_.SetReadDuration(base_duration);
  config_.SetWriteDuration(base_duration);
  config_.SetRewindDuration(base_duration * 10);
}

void TapeSorterBenchmark::SetUpMemoryLimit(const std::size_t value_count, const uint64_t divider) {
  config_.SetMemoryLimit(value_count * sizeof(Value) / divider);
}

void TapeSorterBenchmark::SetUpPerThreadCount(const std::size_t value_count) {
  config_.SetMaxValueCountPerThread(value_count / std::thread::hardware_concurrency());
}

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortRandomArray)(::benchmark::State &state) {
  const auto value_count = state.range(0);
  SetUpMemoryLimit(value_count);
  SetUpPerThreadCount(value_count);
  const auto values = InitInputDataWithRandomValues(value_count);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortRandomArray)->Arg(5000)->Arg(10000)->Arg(25000);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortSortedArray)(::benchmark::State &state) {
  const size_t value_count = state.range(0);
  SetUpMemoryLimit(value_count);
  SetUpPerThreadCount(value_count);
  auto expected_values = GenerateRandomArray<Value>(value_count);
  std::ranges::sort(expected_values);
  CreateFileWithBinaryContent(input_file_path_, expected_values);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortSortedArray)->Arg(5000)->Arg(10000)->Arg(25000);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortSortedInReverseOrderArray)(::benchmark::State &state) {
  const size_t value_count = state.range(0);
  SetUpMemoryLimit(value_count);
  SetUpPerThreadCount(value_count);
  auto expected_values = GenerateRandomArray<Value>(value_count);
  std::ranges::sort(expected_values, std::greater());
  CreateFileWithBinaryContent(input_file_path_, expected_values);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortSortedInReverseOrderArray)
    ->Arg(5000)
    ->Arg(10000)
    ->Arg(25000);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortArrayWithMemoryLimit)(::benchmark::State &state) {
  constexpr size_t value_count = 40000;
  SetUpMemoryLimit(value_count, state.range(0));
  SetUpPerThreadCount(value_count);
  const auto values = InitInputDataWithRandomValues(value_count);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortArrayWithMemoryLimit)
    ->Arg(1)
    ->Arg(5)
    ->Arg(20)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortArrayWithDifferentDelays)(::benchmark::State &state) {
  constexpr size_t value_count = 20000;
  SetUpMemoryLimit(value_count);
  SetUpPerThreadCount(value_count);
  SetUpTapeDurations(state.range(0) * 1us);
  const auto values = InitInputDataWithRandomValues(value_count);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortArrayWithDifferentDelays)
    ->Arg(1)
    ->Arg(10)
    ->Arg(20)
    ->Arg(100);

BENCHMARK_MAIN();

}  // namespace sot::test::benchmark
