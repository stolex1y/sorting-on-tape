#include <benchmark/benchmark.h>

#include "tape_sorter_test_base.h"

namespace sot::test::benchmark {

using Value = std::int64_t;

class TapeSorterBenchmark : public TapeSorterTestBase<Value>, public ::benchmark::Fixture {
 public:
  void SetUp(::benchmark::State &st) override;
  void TearDown(::benchmark::State &st) override;
  void SetUpTapeDurations();
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

void TapeSorterBenchmark::SetUpTapeDurations() {
  config_.SetMoveDuration(1ms);
  config_.SetReadDuration(1ms);
  config_.SetWriteDuration(1ms);
  config_.SetRewindDuration(1ms);
}

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortRandomArray)(::benchmark::State &state) {
  const auto numbers = InitInputDataWithRandomNumbers(state.range(0));
  for (auto _ : state) {
    const auto sorted_numbers = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortRandomArray)->RangeMultiplier(8)->Range(16, 1024);

BENCHMARK_DEFINE_F(TapeSorterBenchmark, SortSortedArray)(::benchmark::State &state) {
  const auto numbers = InitInputDataWithRandomNumbers(state.range(0));
  for (auto _ : state) {
    const auto sorted_numbers = SortTape();
  }
}
BENCHMARK_REGISTER_F(TapeSorterBenchmark, SortRandomArray)->RangeMultiplier(8)->Range(16, 1024);

BENCHMARK_MAIN();

}  // namespace sot::test::benchmark
