#include "file_utils.h"

#include <filesystem>

namespace sot::test::files {

void CreateDir(const std::string& directories) {
  std::filesystem::create_directories(directories);
}

void CreateFileWithContent(const std::string& file_name, const std::string& content) {
  std::fstream file(file_name, std::ios_base::out | std::ios_base::trunc);
  file << content;
}

}  // namespace sot::test::files
