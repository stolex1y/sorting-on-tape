#include "test_utils.h"

#include <gtest/gtest.h>

namespace sot::test {

std::string ReadAllFromTapeAsString(Tape<char> &tape) {
  const auto chars = tape.ReadN(std::string::npos);
  return {chars.begin(), chars.end()};
}

void VerifyContentEquals(const std::string &expected, const std::string &actual) {
  EXPECT_EQ(expected, actual) << "Actual content is different";
}

}  // namespace sot::test
