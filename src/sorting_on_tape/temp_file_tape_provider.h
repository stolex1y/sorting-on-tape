#ifndef FILE_TAPE_PROVIDER_H
#define FILE_TAPE_PROVIDER_H

#include <filesystem>
#include <random>

#include "configuration.h"
#include "file_tape.h"
#include "temp_tape_provider.h"

namespace sot {
/**
 * \brief Класс, предоставляющий временные файловые реализации ленточных устройств.
 *
 * Создает файлы для устройств в системном временном каталоге.
 *
 * \warning При уничтожении экзмепляра данного класса, файлы устройств также удаляются.
 *
 * \tparam Value тип элементов ленты.
 */
template <typename Value>
class TempFileTapeProvider : public TempTapeProvider<Value> {
 public:
  /**
   * Конфигурация используется при создании устройств (см. @link FileTape @endlink).
   */
  explicit TempFileTapeProvider(const Configuration &config);
  /**
   * \brief Очищает созданный каталог для устройств.
   */
  ~TempFileTapeProvider() override;

  [[nodiscard]] std::unique_ptr<Tape<Value>> Get() const override;

 private:
  inline static std::mt19937_64 generator_{std::random_device()()};

  const Configuration &config_;
  const std::filesystem::path prefix_;

  /**
   * \brief Сгенерировать случайный индекс файла или каталога.
   */
  static uint64_t GenerateRandomIndex();
};

template <typename Value>
TempFileTapeProvider<Value>::TempFileTapeProvider(const Configuration &config)
    : config_(config),
      prefix_(
          std::filesystem::temp_directory_path() /
          (std::to_string(GenerateRandomIndex()) + "_tapes")
      ) {
  create_directories(prefix_);
}

template <typename Value>
TempFileTapeProvider<Value>::~TempFileTapeProvider() {
  remove_all(prefix_);
}

template <typename Value>
std::unique_ptr<Tape<Value>> TempFileTapeProvider<Value>::Get() const {
  return std::make_unique<FileTape<Value>>(
      config_, prefix_ / std::to_string(GenerateRandomIndex())
  );
}

template <typename Value>
uint64_t TempFileTapeProvider<Value>::GenerateRandomIndex() {
  return generator_();
}

}  // namespace sot

#endif  // FILE_TAPE_PROVIDER_H
