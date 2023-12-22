#ifndef TAPE_BLOCK_WRITER_H
#define TAPE_BLOCK_WRITER_H

#include <memory>

#include "tape.h"

namespace sot {

/**
 * \brief Класс, позволяющий выполнять запись на ленту с встроенной буфферизацией.
 * \tparam Value тип элемента, хранимого на ленте.
 */
template <typename Value>
class TapeBlockWriter {
 public:
  TapeBlockWriter(size_t capacity, std::shared_ptr<Tape<Value>> tape);
  TapeBlockWriter(TapeBlockWriter &&other) = default;
  TapeBlockWriter &operator=(TapeBlockWriter &&other) = default;
  ~TapeBlockWriter();

  /**
   * \brief Выполнить запись значения на текущей позиции и сдвинуть курсор вперед.
   */
  void Write(Value value);
  /**
   * \brief Принудительно отправить данные из буфера и очистить его.
   */
  void Flush();

 private:
  using TapeSharedPtr = std::shared_ptr<Tape<Value>>;

  size_t capacity_;
  TapeSharedPtr tape_;
  std::vector<Value> values_;
  size_t pos_ = 0;

  /**
   * \brief Записать буфер данных и очистить его.
   */
  void WriteBlock();
  /**
   * \brief Сдвинуть курсор на одну позицию вперед.
   */
  void MoveForward();
};

template <typename Value>
TapeBlockWriter<Value>::TapeBlockWriter(const size_t capacity, std::shared_ptr<Tape<Value>> tape)
    : capacity_(capacity), tape_(std::move(tape)) {
  if (capacity == 0) {
    throw std::runtime_error("Capacity must be positive.");
  }
  values_.reserve(capacity);
}

template <typename Value>
TapeBlockWriter<Value>::~TapeBlockWriter() {
  Flush();
}

template <typename Value>
void TapeBlockWriter<Value>::Write(Value value) {
  values_.emplace_back(std::move(value));
  MoveForward();
}

template <typename Value>
void TapeBlockWriter<Value>::Flush() {
  WriteBlock();
}

template <typename Value>
void TapeBlockWriter<Value>::WriteBlock() {
  const auto written = tape_->WriteN(values_);
  if (written != values_.size()) {
    throw std::runtime_error("Can't write to tape.");
  }
  values_.clear();
  pos_ = 0;
}

template <typename Value>
void TapeBlockWriter<Value>::MoveForward() {
  ++pos_;
  if (pos_ > capacity_) {
    WriteBlock();
  }
}

}  // namespace sot

#endif  // TAPE_BLOCK_WRITER_H
