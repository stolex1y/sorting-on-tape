#include "test_utils.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>

namespace sot::test {

std::vector<std::int64_t> GenerateRandomSequence(const size_t size, const int64_t seed) {
  std::vector<std::int64_t> numbers(size, 0);
  std::ranges::generate(numbers, [generator = std::mt19937_64(seed)]() mutable {
    return static_cast<std::int64_t>(generator());
  });
  return numbers;
}

std::string ReadAllFromTape(Tape<char> &tape) {
  const auto chars = tape.ReadN(std::string::npos);
  return {chars.begin(), chars.end()};
}

void VerifyContentEquals(const std::string &expected, const std::string &actual) {
  EXPECT_EQ(expected, actual) << "Actual content is different";
}

}  // namespace sot::test
