#include "file_utils.h"

#include <filesystem>

namespace sot::test::files {

void CreateDirWithReplace(const std::filesystem::path &directories) {
  if (exists(directories)) {
    remove_all(directories);
  }
  create_directories(directories);
}

void CreateFileWithBinaryContent(
    const std::filesystem::path &file_name, const std::string &content
) {
  std::fstream file(file_name, std::ios_base::out | std::ios_base::trunc);
  file << content;
}

}  // namespace sot::test::files
