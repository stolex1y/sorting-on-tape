#ifndef TAPE_SORTING_H
#define TAPE_SORTING_H

#include <algorithm>
#include <execution>
#include <format>
#include <semaphore>

#include "configuration.h"
#include "core/concurrent_queue.h"
#include "core/thread_pool.h"
#include "file_tape.h"
#include "memory_literals.h"
#include "tape.h"
#include "temp_tape_provider.h"

namespace sot {

using namespace memory_literals;

/**
 * \brief Класс, реализующий сортировку данных с входной ленты на выходную.
 * \tparam Value тип элементов ленты.
 * \tparam ThreadPool пул потоков, задействованных при выполнении сортировки.
 */
template <typename Value, typename ThreadPool = core::ThreadPool>
class TapeSorter {
  // using Value = int32_t;
  // using ThreadPool = core::ThreadPool;

 public:
  /// Ключ в конфигурации, задающий лимит использования занимаемой памяти при сортировке в байтах.
  static constexpr auto kMemoryLimitKey = "memory_limit";
  /// Ключ в конфигурации, задающий количество потоков, используемых при сортировке.
  static constexpr auto kMaxThreadCountKey = "max_thread_count";
  /// Ключ в конфигурации, задающий максимальное количество обрабатываемых значений одним потоком.
  static constexpr auto kMaxValueCountPerThreadKey = "max_value_count_per_thread";
  /// Значение лимита использования занимаемой памяти при сортировке по умолчанию.
  static constexpr size_t kDefaultMemoryLimit = 1_GiB;
  /// Значение количества потоков, используемых при сортировке, по умолчанию hardware_concurrency.
  static const size_t kDefaultMaxThreadCount;
  /// Максимальное количество элементов, которое может обрабатывать один поток.
  static constexpr size_t kDefaultMaxValueCountPerThread = 1000000;

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

  class ParallelSortContext {
   public:
    ThreadPool thread_pool;
    /// Определяет количество свободных для выполнения потоков.
    std::counting_semaphore<> free_threads;
    /// Разделенные и отсортированные блоки данных.
    core::ConcurrentQueue<TapeUniquePtr> sorted_blocks;

    explicit ParallelSortContext(const TapeSorter &sorter);
  };

  /// Количество потоков, которое будет задействовано при сортировке.
  size_t thread_count_;
  /// Максимальное количество элементов, которое может содержаться в памяти с учетом ограничений.
  const size_t values_in_memory_limit_;
  /// Максимальное количество элементов, которые будет обрабатывать каждый поток с учетом
  /// ограничений.
  size_t values_per_thread_;
  /// Провайдер временных лент для хранения промежуточных данных.
  const std::shared_ptr<TempTapeProvider<Value>> tape_provider_;

  /**
   * \brief Выполнить сортировку и запись блока на временную ленту.
   *
   * Результирующая лента сохраняется в переданном контексте.
   *
   * \param context контекст выполнения сортировки;
   * \param block_values значения блока;
   */
  void SortAndWriteBlock(ParallelSortContext &context, std::vector<Value> &&block_values) const;
  /**
   * \brief Выполнить слияние двух отсортированных частей массива, записанных на ленты.
   *
   * Результат слияния двух частей сохраняется в переданном контексте.
   *
   * \param context контекст выполнения сортировки;
   * \param first_tape первая часть массива, записанная на ленту;
   * \param second_tape вторая часть массива, записанная на ленту;
   */
  void MergeTwoTapes(
      ParallelSortContext &context,
      const TapeSharedPtr &first_tape,
      const TapeSharedPtr &second_tape
  ) const;
  /**
   * \brief Записать оставшиеся элементы с одного устройства на другое.
   * \param[in] src источник элементов;
   * \param[in] block_size размер блока, переносимого за раз;
   * \param[out] target целевое устройство, куда будут записаны элементы.
   */
  void WriteLeftPart(Tape<Value> &src, size_t block_size, Tape<Value> &target) const;
};

template <typename Value, typename ThreadPool>
const size_t TapeSorter<Value, ThreadPool>::kDefaultMaxThreadCount =
    std::thread::hardware_concurrency();
// 1;

template <typename Value, typename ThreadPool>
TapeSorter<Value, ThreadPool>::TapeSorter(
    const Configuration &config, std::shared_ptr<TempTapeProvider<Value>> tape_provider
)
    : values_in_memory_limit_(
          config.GetProperty(kMemoryLimitKey, kDefaultMemoryLimit) / sizeof(Value)
      ),
      tape_provider_(std::move(tape_provider)) {
  if (values_in_memory_limit_ < 4) {
    throw std::invalid_argument(
        std::format("Increase memory limit! Minimum - {} bytes", sizeof(Value) * 4)
    );
  }

  const auto max_value_count_per_thread =
      config.GetProperty(kMaxValueCountPerThreadKey, kDefaultMaxValueCountPerThread);
  values_per_thread_ = std::min(max_value_count_per_thread, values_in_memory_limit_);

  const auto max_thread_count = config.GetProperty(kMaxThreadCountKey, kDefaultMaxThreadCount);
  thread_count_ = std::min(max_thread_count, values_in_memory_limit_ / values_per_thread_);
}

template <typename Value, typename ThreadPool>
void TapeSorter<Value, ThreadPool>::Sort(Tape<Value> &input_tape, Tape<Value> &output_tape) const {
  ParallelSortContext context(*this);
  // прочитать входные данные поблочно, ввиду ограничения использования памяти
  auto temp_tape_values = input_tape.ReadN(values_per_thread_);  // элементы очередного блока
  while (!temp_tape_values.empty()) {
    context.free_threads.acquire();
    context.thread_pool.PostTask([this, block_values = std::move(temp_tape_values), &context](
                                 ) mutable {
      SortAndWriteBlock(context, std::move(block_values));
      context.free_threads.release();
    });
    temp_tape_values = input_tape.ReadN(values_per_thread_);  // прочитать следующий блок данных
  }
  // выполнить попарное слияние блоков данных, пока не останется 1 блок
  while (context.thread_pool.HasWorks() || context.sorted_blocks.Size() > 1) {
    if (context.sorted_blocks.Size() > 1) {
      do {
        TapeSharedPtr first_tape = *context.sorted_blocks.Pop();
        TapeSharedPtr second_tape = *context.sorted_blocks.Pop();
        context.free_threads.acquire();
        context.thread_pool.PostTask(
            [this, first_tape = first_tape, second_tape = second_tape, &context]() mutable {
              MergeTwoTapes(context, first_tape, second_tape);
              context.free_threads.release();
            }
        );
      } while (context.sorted_blocks.Size() > 1);
    } else {
      std::this_thread::yield();
    }
  }
  const auto sorted_tape = context.sorted_blocks.Pop();
  if (sorted_tape) {
    WriteLeftPart(*sorted_tape.value(), values_in_memory_limit_, output_tape);
    output_tape.MoveToBegin();
  }
}

template <typename Value, typename ThreadPool>
TapeSorter<Value, ThreadPool>::ParallelSortContext::ParallelSortContext(const TapeSorter &sorter)
    : thread_pool(sorter.thread_count_), free_threads(sorter.thread_count_) {
}

template <typename Value, typename ThreadPool>
void TapeSorter<Value, ThreadPool>::SortAndWriteBlock(
    ParallelSortContext &context, std::vector<Value> &&block_values
) const {
  TapeUniquePtr tape = tape_provider_->Get();  // временное устройство для хранения очередного блока
  std::ranges::sort(block_values);
  tape->WriteN(block_values);
  tape->MoveToBegin();
  context.sorted_blocks.Push(std::move(tape));
}

template <typename Value, typename ThreadPool>
void TapeSorter<Value, ThreadPool>::MergeTwoTapes(
    ParallelSortContext &context, const TapeSharedPtr &first_tape, const TapeSharedPtr &second_tape
) const {
  auto merged_tape = tape_provider_->Get();  // устройство для объединенных данных
  // слияние будет выполняться по частям указанного размера
  const auto part_size = values_per_thread_ / 4;

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
    WriteLeftPart(*first_tape, values_per_thread_, *merged_tape);
  } else {
    // записать оставшуюся часть из второго устройства
    merged_tape->WriteN(second_part.begin() + second_i, second_part.end());
    second_part.clear();
    WriteLeftPart(*second_tape, values_per_thread_, *merged_tape);
  }
  merged_tape->MoveToBegin();
  context.sorted_blocks.Push(std::move(merged_tape));
}

template <typename Value, typename ThreadPool>
void TapeSorter<Value, ThreadPool>::WriteLeftPart(
    Tape<Value> &src, const size_t block_size, Tape<Value> &target
) const {
  auto values = src.ReadN(block_size);
  while (!values.empty()) {
    target.WriteN(values);
    values = src.ReadN(block_size);
  }
}

}  // namespace sot

#endif  // TAPE_SORTING_H
