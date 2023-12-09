#ifndef FILE_CREATOR_H
#define FILE_CREATOR_H

#include <fstream>
#include <string>

namespace sot::test::files {

/**
 * \brief Создать директории.
 */
void CreateDir(const std::string &directories);
/**
 * \brief Создать файл, заполненный указанным содержимым.
 */
void CreateFileWithContent(const std::string &file_name, const std::string &content);

}  // namespace sot::test::files

#endif  // FILE_CREATOR_H
