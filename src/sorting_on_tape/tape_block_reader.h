#ifndef TAPE_BLOCK_READER_H
#define TAPE_BLOCK_READER_H

#include <memory>

#include "tape.h"

namespace sot {

/**
 * \brief Класс, позволяющий выполнять считывание с ленты с встроенной буфферизацией.
 * \tparam Value тип элемента, хранимого на ленте.
 */
template <typename Value>
class TapeBlockReader {
 public:
  TapeBlockReader(size_t capacity, std::shared_ptr<Tape<Value>> tape);
  TapeBlockReader(TapeBlockReader &&other) = default;
  TapeBlockReader &operator=(TapeBlockReader &&other) = default;

  /**
   * \brief Сдвинуть курсор вперед на одну позицию.
   * \return false - если достигнут конец устройства.
   */
  bool MoveForward();
  /**
   * \brief Прочитать значение на текущей позиции.
   */
  Value Read() const;

 private:
  using TapeSharedPtr = std::shared_ptr<Tape<Value>>;

  size_t capacity_;
  TapeSharedPtr tape_;
  std::vector<Value> values_;
  size_t pos_ = 0;

  /**
   * \brief Выполнить чтение следующего блока с устройства.
   */
  void ReadNextBlock();
};

template <typename Value>
TapeBlockReader<Value>::TapeBlockReader(const size_t capacity, std::shared_ptr<Tape<Value>> tape)
    : capacity_(capacity), tape_(std::move(tape)) {
  if (capacity == 0) {
    throw std::runtime_error("Capacity must be positive.");
  }
  ReadNextBlock();
}

template <typename Value>
bool TapeBlockReader<Value>::MoveForward() {
  ++pos_;
  if (pos_ == values_.size()) {
    ReadNextBlock();
    return !values_.empty();
  }
  return true;
}

template <typename Value>
Value TapeBlockReader<Value>::Read() const {
  if (pos_ >= values_.size()) {
    throw std::runtime_error("Tried to read out of bounds.");
  }
  return values_[pos_];
}

template <typename Value>
void TapeBlockReader<Value>::ReadNextBlock() {
  values_ = tape_->ReadN(capacity_);
  pos_ = 0;
}

}  // namespace sot

#endif  // TAPE_BLOCK_READER_H
