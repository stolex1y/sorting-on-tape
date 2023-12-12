#ifndef TEST_BASE_H
#define TEST_BASE_H

#include <filesystem>

#include "fake_configuration.h"
#include "file_utils.h"

namespace sot::test {

using std::filesystem::path;
using namespace files;

/**
 * \brief Базовый класс для тестов, связанных с @link FileTape @endlink.
 */
class TestBase {
 public:
  FakeConfiguration config_;
  std::string test_name_;
  path file_prefix_;

  virtual ~TestBase() = default;

  virtual void SetUp(const std::string &test_suit_name, const std::string &test_name);
};

}  // namespace sot::test

#endif  // TEST_BASE_H
