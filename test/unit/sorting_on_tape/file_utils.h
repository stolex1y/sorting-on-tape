#ifndef FILE_CREATOR_H
#define FILE_CREATOR_H

#include <filesystem>
#include <fstream>
#include <string>

#include "test_utils.h"

namespace sot::test::files {

/**
 * \brief Создать директории.
 *
 * При этом, если они уже существуют, то сначала происходит удаление этих директорий и их
 * содержимого.
 */
void CreateDirWithReplace(const std::filesystem::path &directories);
/**
 * \brief Создать файл, заполненный указанным содержимым.
 */
void CreateFileWithBinaryContent(
    const std::filesystem::path &file_name, const std::string &content
);

/**
 * \brief Создать файл, заполненный указанным содержимым.
 *
 * Запись происходит в двоичном режиме.
 */
template <typename Values>
void CreateFileWithBinaryContent(const std::filesystem::path &file_name, const Values &values) {
  std::fstream file(file_name, std::ios_base::out | std::ios_base::trunc);
  for (const auto &value : values) {
    const auto bytes = reinterpret_cast<const char *>(&value);
    file.write(bytes, sizeof(value));
  }
}

/**
 * \brief Создать файл, заполненный указанным содержимым.
 *
 * Запись происходит в текстовом режиме.
 */
template <typename Values>
void CreateFileWithContent(const std::filesystem::path &file_name, const Values &values) {
  std::fstream file(file_name, std::ios_base::out | std::ios_base::trunc);
  file << values;
}

}  // namespace sot::test::files

#endif  // FILE_CREATOR_H
