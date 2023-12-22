#ifndef TAPE_SORTING_H
#define TAPE_SORTING_H

#include <algorithm>
#include <execution>
#include <format>
#include <queue>

#include "configuration.h"
#include "core/thread_pool.h"
#include "memory_literals.h"
#include "tape.h"
#include "tape_block_reader.h"
#include "tape_block_writer.h"
#include "temp_tape_provider.h"

namespace sot {

using namespace memory_literals;

/**
 * \brief Класс, реализующий сортировку данных с входной ленты на выходную.
 * \tparam Value тип элементов ленты.
 * \tparam ThreadPool пул потоков, задействованных при выполнении сортировки.
 * \tparam Comparator компаратор, по умолчанию используется std::less<Value>.
 */
template <
    typename Value,
    typename Comparator = std::less<Value>,
    typename ThreadPool = core::ThreadPool>
class TapeSorter {
 public:
  /// Ключ в конфигурации, задающий лимит использования занимаемой памяти при сортировке в байтах.
  static constexpr auto kMemoryLimitKey = "memory_limit";
  /// Ключ в конфигурации, задающий количество потоков, используемых при сортировке.
  static constexpr auto kMaxThreadCountKey = "max_thread_count";
  /// Ключ в конфигурации, задающий максимальное количество обрабатываемых значений одним потоком.
  static constexpr auto kMaxValueCountPerThreadKey = "max_value_count_per_thread";
  /// Ключ в конфигурации, задающий максимальное количество блоков, сливаемых одновременно.
  static constexpr auto kMaxMerginGroupSizeKey = "max_merging_group_size";
  /// Значение лимита использования занимаемой памяти при сортировке по умолчанию.
  static constexpr size_t kDefaultMemoryLimit = 1_GiB;
  /// Значение количества потоков, используемых при сортировке, по умолчанию hardware_concurrency.
  static const size_t kDefaultMaxThreadCount;
  /// Максимальное количество элементов, которое может обрабатывать один поток.
  static constexpr size_t kDefaultMaxValueCountPerThread = 1000000;
  /// Максимальное количество блоков, сливаемых одновременно.
  static constexpr size_t kDefaultMaxMergingGroupSize = 50;

  TapeSorter(
      const Configuration &config,
      std::shared_ptr<TempTapeProvider<Value>> tape_provider,
      Comparator comparator = Comparator()
  );

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
    /// Количество оставшихся блоков, после завершения запущенных слияний.
    std::atomic_size_t block_count;
    /// Количество значений, которые может обрабатывать один поток.
    std::atomic_size_t values_per_thread;

    explicit ParallelSortContext(const TapeSorter &sorter);

    /**
     * \brief Добавить отсортированный блок.
     */
    void Push(TapeSharedPtr tape);
    /**
     * \brief Удалить первый блок.
     */
    TapeSharedPtr Pop();
    /**
     * \brief Удалить заданное количество первых блоков для их слияния.
     */
    std::vector<TapeSharedPtr> PopBlocksToMerge();
    [[nodiscard]] bool Empty() const;

   private:
    const size_t merging_group_size_;
    /// Разделенные и отсортированные блоки данных.
    std::queue<TapeSharedPtr> queue_;
    std::condition_variable has_blocks_to_merge_;
    std::condition_variable has_blocks_;
    mutable std::mutex mutex_;

    /**
     * \brief Проверка наличия нужного количества блоков для выполнения слияния.
     */
    [[nodiscard]] bool HasBlocksToMerge() const;
    /**
     * \brief Незащищенное удаление блока.
     */
    TapeSharedPtr Pop_();
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
  Comparator comparator_;
  /// Максимальное количество блоков, сливаемых одновременно.
  const size_t merging_group_size_;

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
   * \brief Выполнить слияние нескольких отсортированных частей массива, записанных на ленты.
   *
   * Результат слияния сохраняется в переданном контексте.
   *
   * \param context контекст выполнения сортировки;
   * \param tapes сливаемые части массива.
   */
  void MergeTapes(ParallelSortContext &context, const std::vector<TapeSharedPtr> &tapes) const;
  /**
   * \brief Записать оставшиеся элементы с одного устройства на другое.
   * \param[in] src источник элементов;
   * \param[in] block_size размер блока, переносимого за раз;
   * \param[out] target целевое устройство, куда будут записаны элементы.
   */
  void WriteLeftPart(Tape<Value> &src, size_t block_size, Tape<Value> &target) const;
};

template <typename Value, typename Comparator, typename ThreadPool>
const size_t TapeSorter<Value, Comparator, ThreadPool>::kDefaultMaxThreadCount =
    std::thread::hardware_concurrency();

template <typename Value, typename Comparator, typename ThreadPool>
TapeSorter<Value, Comparator, ThreadPool>::TapeSorter(
    const Configuration &config,
    std::shared_ptr<TempTapeProvider<Value>> tape_provider,
    Comparator comparator
)
    : values_in_memory_limit_(
          config.GetProperty(kMemoryLimitKey, kDefaultMemoryLimit) / sizeof(Value)
      ),
      tape_provider_(std::move(tape_provider)),
      comparator_(comparator),
      merging_group_size_(config.GetProperty(kMaxMerginGroupSizeKey, kDefaultMaxMergingGroupSize)) {
  if (values_in_memory_limit_ < 4) {
    throw std::invalid_argument(
        std::format("Increase memory limit! Minimum - {} bytes.", sizeof(Value) * 4)
    );
  }

  const auto max_value_count_per_thread =
      config.GetProperty(kMaxValueCountPerThreadKey, kDefaultMaxValueCountPerThread);
  values_per_thread_ = std::min(max_value_count_per_thread, values_in_memory_limit_);

  if (values_per_thread_ / (merging_group_size_ + 1) < 1) {
    throw std::invalid_argument(std::format(
        "Can't merge {} blocks in thread! Increase memory limit or max value count per thread - "
        "need minimum {} bytes.",
        merging_group_size_,
        (merging_group_size_ + 1) * sizeof(Value)
    ));
  }

  const auto max_thread_count = config.GetProperty(kMaxThreadCountKey, kDefaultMaxThreadCount);
  thread_count_ = std::min(max_thread_count, values_in_memory_limit_ / values_per_thread_);
}

template <typename Value, typename Comparator, typename ThreadPool>
void TapeSorter<Value, Comparator, ThreadPool>::Sort(
    Tape<Value> &input_tape, Tape<Value> &output_tape
) const {
  ParallelSortContext context(*this);
  // прочитать входные данные поблочно, ввиду ограничения использования памяти
  auto temp_tape_values = input_tape.ReadN(context.values_per_thread);  // элементы очередного блока
  while (!temp_tape_values.empty()) {
    ++context.block_count;
    context.thread_pool.PostTask([this, block_values = std::move(temp_tape_values), &context](
                                 ) mutable {
      SortAndWriteBlock(context, std::move(block_values));
    });
    temp_tape_values =
        input_tape.ReadN(context.values_per_thread);  // прочитать следующий блок данных
  }
  // выполнить попарное слияние блоков данных, пока не останется 1 блок
  while (context.block_count > 1) {
    std::vector<TapeSharedPtr> blocks_to_merge = context.PopBlocksToMerge();
    const size_t merged = blocks_to_merge.size();
    context.thread_pool.PostTask([this, tapes = std::move(blocks_to_merge), &context]() mutable {
      MergeTapes(context, tapes);
    });
    context.block_count.fetch_sub(merged - 1);
  }
  if (context.block_count > 0) {
    const auto sorted = context.Pop();
    WriteLeftPart(*sorted, values_in_memory_limit_, output_tape);
    output_tape.MoveToBegin();
  }
}

template <typename Value, typename Comparator, typename ThreadPool>
TapeSorter<Value, Comparator, ThreadPool>::ParallelSortContext::ParallelSortContext(
    const TapeSorter &sorter
)
    : thread_pool(sorter.thread_count_),
      values_per_thread(sorter.values_per_thread_),
      merging_group_size_(sorter.merging_group_size_) {
}

template <typename Value, typename Comparator, typename ThreadPool>
void TapeSorter<Value, Comparator, ThreadPool>::ParallelSortContext::Push(TapeSharedPtr tape) {
  std::lock_guard lock(mutex_);
  queue_.emplace(std::move(tape));
  if (HasBlocksToMerge()) {
    has_blocks_to_merge_.notify_one();
  }
  has_blocks_.notify_one();
}

template <typename Value, typename Comparator, typename ThreadPool>
typename TapeSorter<Value, Comparator, ThreadPool>::TapeSharedPtr
TapeSorter<Value, Comparator, ThreadPool>::ParallelSortContext::Pop() {
  std::unique_lock lock(mutex_);
  has_blocks_.wait(lock, [this] {
    return !queue_.empty();
  });
  return Pop_();
}

template <typename Value, typename Comparator, typename ThreadPool>
std::vector<typename TapeSorter<Value, Comparator, ThreadPool>::TapeSharedPtr>
TapeSorter<Value, Comparator, ThreadPool>::ParallelSortContext::PopBlocksToMerge() {
  std::unique_lock lock(mutex_);
  has_blocks_to_merge_.wait(lock, [this] {
    return HasBlocksToMerge();
  });
  std::vector<TapeSharedPtr> tapes(std::min(merging_group_size_, block_count.load()));
  for (size_t i = 0; i < tapes.size(); ++i) {
    tapes[i] = Pop_();
  }
  return tapes;
}

template <typename Value, typename Comparator, typename ThreadPool>
bool TapeSorter<Value, Comparator, ThreadPool>::ParallelSortContext::Empty() const {
  std::lock_guard lock(mutex_);
  return queue_.empty();
}

template <typename Value, typename Comparator, typename ThreadPool>
bool TapeSorter<Value, Comparator, ThreadPool>::ParallelSortContext::HasBlocksToMerge() const {
  return queue_.size() >= std::min(block_count.load(), merging_group_size_);
}

template <typename Value, typename Comparator, typename ThreadPool>
typename TapeSorter<Value, Comparator, ThreadPool>::TapeSharedPtr
TapeSorter<Value, Comparator, ThreadPool>::ParallelSortContext::Pop_() {
  TapeSharedPtr value = std::move(queue_.front());
  queue_.pop();
  return value;
}

template <typename Value, typename Comparator, typename ThreadPool>
void TapeSorter<Value, Comparator, ThreadPool>::SortAndWriteBlock(
    ParallelSortContext &context, std::vector<Value> &&block_values
) const {
  auto tape = tape_provider_->Get();  // временное устройство для хранения очередного блока
  std::sort(block_values.begin(), block_values.end(), comparator_);
  tape->WriteN(block_values);
  tape->MoveToBegin();
  context.Push(std::move(tape));
}

template <typename Value, typename Comparator, typename ThreadPool>
void TapeSorter<Value, Comparator, ThreadPool>::MergeTapes(
    ParallelSortContext &context, const std::vector<TapeSharedPtr> &tapes
) const {
  using BlockReader = TapeBlockReader<Value>;
  using BlockWriter = TapeBlockWriter<Value>;

  // слияние будет выполняться по частям указанного размера
  const auto block_size = context.values_per_thread / (tapes.size() + 1);

  // устройство для объединенных данных
  TapeSharedPtr merged_tape = tape_provider_->Get();
  BlockWriter merged_block(block_size, merged_tape);

  const auto block_comparator = [this](const BlockReader &f, const BlockReader &s) {
    return f.Read() != s.Read() && !comparator_(f.Read(), s.Read());
  };
  std::priority_queue<BlockReader, std::vector<BlockReader>, decltype(block_comparator)> blocks(
      block_comparator
  );

  for (const auto &tape : tapes) {
    blocks.emplace(block_size, tape);
  }
  while (!blocks.empty()) {
    BlockReader block = std::move(const_cast<BlockReader &>(blocks.top()));
    blocks.pop();
    merged_block.Write(block.Read());
    if (block.MoveForward()) {
      blocks.emplace(std::move(block));
    }
  }
  merged_block.Flush();
  merged_tape->MoveToBegin();
  context.Push(std::move(merged_tape));
}

template <typename Value, typename Comparator, typename ThreadPool>
void TapeSorter<Value, Comparator, ThreadPool>::WriteLeftPart(
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
