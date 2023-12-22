#include <benchmark/benchmark.h>

#include "tape_sorter_test_base.h"

namespace sot::test::benchmark {

using namespace ::benchmark;

using Value = std::int32_t;

class TapeSorterBenchmark : public TapeSorterTestBase<Value>, public Fixture {
 public:
  void SetUp(State &st) override;
  void TearDown(State &st) override;
  void SetUpTapeDurations(size_t multiplier = 1);
  void SetUpMemoryLimit(std::size_t value_count, uint64_t divider = 10);
  void SetUpPerThreadCount(
      std::size_t value_count, std::size_t thread_count = std::thread::hardware_concurrency()
  );
};

void TapeSorterBenchmark::SetUp(State &st) {
  Fixture::SetUp(st);
  const auto benchmark_name = st.name();
  const auto test_suit_name = benchmark_name.substr(0, benchmark_name.find('/'));
  const auto test_name = benchmark_name.substr(benchmark_name.find('/') + 1);
  TapeSorterTestBase::SetUp(test_suit_name, test_name);
  SetUpTapeDurations();
}

void TapeSorterBenchmark::TearDown(State &st) {
  Fixture::TearDown(st);
}

void TapeSorterBenchmark::SetUpTapeDurations(const size_t multiplier) {
  const auto base = 1us;
  config_.SetMoveDuration(base * multiplier);
  config_.SetReadDuration(base * multiplier * 5);
  config_.SetWriteDuration(base * multiplier * 5);
  config_.SetRewindDuration(base * multiplier * 10000);
  config_.SetGapCrossDuration(base * multiplier * 1000);
}

void TapeSorterBenchmark::SetUpMemoryLimit(const std::size_t value_count, const uint64_t divider) {
  config_.SetMemoryLimit(value_count * sizeof(Value) / divider);
}

void TapeSorterBenchmark::SetUpPerThreadCount(
    const std::size_t value_count, const std::size_t thread_count
) {
  config_.SetMaxValueCountPerThread(value_count / thread_count);
}

BENCHMARK_DEFINE_F(TapeSorterBenchmark, WithMemoryLimit)(State &state) {
  constexpr auto value_count = 15000;
  const auto mem_divider = state.range(0);
  const auto thread_count = state.range(1);
  SetUpMemoryLimit(value_count, mem_divider);
  config_.SetMaxThreadCount(thread_count);
  config_.SetMaxValueCountPerThread(value_count / mem_divider / thread_count);
  config_.SetMaxMergingGroupSize(2);
  const auto values = InitInputDataWithRandomValues(value_count);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, WithMemoryLimit)
    ->ArgNames({"memory_divider", "threads"})
    ->ArgsProduct({{10, 100, 500}, {1, 2, 8}});

BENCHMARK_DEFINE_F(TapeSorterBenchmark, WithDifferentMergeGroupSize)(State &state) {
  constexpr auto value_count = 100000;
  constexpr auto mem_divider = 100;
  const auto merge_group_size = state.range(0);
  const auto thread_count = state.range(1);
  SetUpMemoryLimit(value_count, mem_divider);
  config_.SetMaxThreadCount(thread_count);
  config_.SetMaxValueCountPerThread(value_count / mem_divider / thread_count);
  config_.SetMaxMergingGroupSize(merge_group_size);
  const auto values = InitInputDataWithRandomValues(value_count);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, WithDifferentMergeGroupSize)
    ->ArgNames({"merge_group_size", "threads"})
    ->ArgsProduct({{2, 5, 10, 15, 20}, {1, 2, 8}});

BENCHMARK_DEFINE_F(TapeSorterBenchmark, WithDifferentDelays)(State &state) {
  constexpr auto value_count = 10000;
  constexpr auto mem_divider = 50;
  constexpr auto merge_group_size = 2;
  const auto delay_multiplier = state.range(0);
  const auto thread_count = state.range(1);
  SetUpTapeDurations(delay_multiplier);
  SetUpMemoryLimit(value_count, mem_divider);
  config_.SetMaxThreadCount(thread_count);
  config_.SetMaxValueCountPerThread(value_count / mem_divider / thread_count);
  config_.SetMaxMergingGroupSize(merge_group_size);
  const auto values = InitInputDataWithRandomValues(value_count);
  for (auto _ : state) {
    const auto sorted_values = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, WithDifferentDelays)
    ->ArgNames({"delay_multiplier", "threads"})
    ->ArgsProduct({{1, 2, 10}, {1, 2, 8}});

BENCHMARK_MAIN();

}  // namespace sot::test::benchmark
