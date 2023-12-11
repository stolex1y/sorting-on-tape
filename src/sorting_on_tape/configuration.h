#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace sot {

/**
 * \brief Класс, выполняющий чтение конфигурации из файла.
 */
class Configuration {
 public:
  /// Имя файла с конфигурацией по умолчанию.
  static constexpr auto kDefaultFileName = "config.properties";

  explicit Configuration(const std::string &file_name = kDefaultFileName);
  Configuration(Configuration &other) = delete;
  Configuration &operator=(Configuration &other) = delete;
  virtual ~Configuration() = default;

  /**
   * \brief Получить значение свойства по ключу либо переданное значение по умолчанию.
   */
  std::uint64_t GetProperty(const std::string &key, std::uint64_t default_value) const;

 protected:
  std::unordered_map<std::string, std::uint64_t> params;

 private:
  /**
   * \brief Прочитать значения параметров из файла.
   */
  void ReadParamsFromFile(const std::string &file_name);
};

}  // namespace sot

#endif  // CONFIGURATION_H
