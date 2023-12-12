#include "test_base.h"

namespace sot::test {

void TestBase::SetUp(const std::string &test_suit_name, const std::string &test_name) {
  test_name_ = test_name;
  file_prefix_ = path(test_suit_name) / test_name;
  CreateDirWithReplace(file_prefix_);
  config_.SetZeroDurations();
}

}  // namespace sot::test
