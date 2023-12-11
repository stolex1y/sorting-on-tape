#include "file_tape.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <random>

#include "fake_configuration.h"
#include "file_utils.h"
#include "test_utils.h"

namespace sot::test {

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace std::filesystem;
using namespace files;

class FileTapeTest : public testing::Test {
 public:
  FakeConfiguration config_;
  std::string test_name_ = testing::UnitTest::GetInstance()->current_test_info()->name();
  path file_prefix_ = path("FileTapeTest") / test_name_;

  FileTapeTest();
};

FileTapeTest::FileTapeTest() {
  CreateDirWithReplace(file_prefix_);
  config_.SetZeroDurations();
}

TEST_F(FileTapeTest, ReadFromMutableFileTape) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content\nTest content";
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char> under_test(config_, file_name);

  const auto actual_content = ReadAllFromTape(under_test);

  VerifyContentEquals(content, actual_content);
}

TEST_F(FileTapeTest, ReadUntilTrue) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  std::string actual_content;
  while (const auto ch = under_test.Read()) {
    actual_content += *ch;
  }

  VerifyContentEquals(content, actual_content);
  VerifyCursorAtTheEnd(under_test);
}

TEST_F(FileTapeTest, ReadFromEmptyTape) {
  const auto file_name = file_prefix_ / "file";
  const std::string content;
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  const auto actual_content = ReadAllFromTape(under_test);

  VerifyContentEquals(content, actual_content);
  VerifyCursorAtTheEnd(under_test);
  VerifyCursorAtTheBeginning<char>(under_test, std::nullopt);
}

TEST_F(FileTapeTest, ReadPartOfConent) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  constexpr auto part_size = 4;
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char> under_test(config_, file_name);

  const auto chars = under_test.ReadN(part_size);
  const std::string actual_content(chars.begin(), chars.end());

  VerifyContentEquals(content.substr(0, part_size), actual_content);
  EXPECT_NE(std::nullopt, under_test.Read())
      << "After reading part of the content, there is nothing left";
}

TEST_F(FileTapeTest, MoveToEnd) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  under_test.MoveToEnd();

  VerifyCursorAtTheEnd(under_test);
}

TEST_F(FileTapeTest, MoveForwardToReadPartOfContent) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  constexpr auto skip = 5;
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char> under_test(config_, file_name);

  for (size_t i = 0; i < skip; ++i) {
    under_test.MoveForward();
  }
  const auto actual_content = ReadAllFromTape(under_test);

  VerifyContentEquals(content.substr(skip), actual_content);
}

TEST_F(FileTapeTest, MoveBackwardFromBegin) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  VerifyCursorAtTheBeginning<char>(under_test, content.front());
}

TEST_F(FileTapeTest, MoveBackwardFromEnd) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  under_test.MoveToEnd();
  under_test.MoveBackward();
  const auto last_char = under_test.Read();

  EXPECT_EQ(content.back(), last_char) << "After moving back, there must be the last character";
}

TEST_F(FileTapeTest, MoveToBegin) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  under_test.MoveToEnd();
  under_test.MoveToBegin();

  VerifyCursorAtTheBeginning<char>(under_test, content.front());
}

TEST_F(FileTapeTest, ReadFullContentTwice) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "Test content";
  CreateFileWithBinaryContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  const auto first_content = ReadAllFromTape(under_test);
  under_test.MoveToBegin();
  const auto second_content = ReadAllFromTape(under_test);

  VerifyContentEquals(content, first_content);
  VerifyContentEquals(content, second_content);
}

TEST_F(FileTapeTest, WriteToImmutableFileTape) {
  const auto file_name = file_prefix_ / "file";
  const std::string before_write = "Test content";
  const std::string new_content = "Updated message";
  CreateFileWithBinaryContent(file_name, before_write);
  FileTape<char, false> under_test(config_, file_name);

  const auto written = under_test.WriteN({new_content.begin(), new_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTape(under_test);

  EXPECT_EQ(0, written) << "Mustn't be written to immutable tape";
  VerifyContentEquals(before_write, actual_content);
}

TEST_F(FileTapeTest, WriteToMutableFileTape) {
  const auto file_name = file_prefix_ / "file";
  const std::string before_write = "Test content";
  const std::string new_content = "Updated message";
  CreateFileWithBinaryContent(file_name, before_write);
  FileTape<char, true> under_test(config_, file_name);

  const auto written = under_test.WriteN({new_content.begin(), new_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTape(under_test);

  EXPECT_EQ(new_content.size(), written);
  VerifyContentEquals(new_content, actual_content);
}

TEST_F(FileTapeTest, WriteToMutableFileTapeUsingIterators) {
  const auto file_name = file_prefix_ / "file";
  const std::string before_write = "Test content";
  const std::string new_content = "Updated message";
  const std::vector new_content_chars(new_content.begin(), new_content.end());
  CreateFileWithBinaryContent(file_name, before_write);
  FileTape<char, true> under_test(config_, file_name);

  const auto last_written = under_test.WriteN(new_content_chars.begin(), new_content_chars.end());
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTape(under_test);

  const auto written = std::distance(new_content_chars.begin(), last_written);
  EXPECT_EQ(new_content.size(), written);
  VerifyContentEquals(new_content, actual_content);
}

TEST_F(FileTapeTest, AppendToMutableFileTape) {
  const auto file_name = file_prefix_ / "file";
  const std::string before_write = "Test content";
  const std::string appended_content = ". Appended content";
  CreateFileWithBinaryContent(file_name, before_write);
  FileTape<char, true> under_test(config_, file_name);

  under_test.MoveToEnd();
  const auto written = under_test.WriteN({appended_content.begin(), appended_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTape(under_test);

  EXPECT_EQ(appended_content.size(), written);
  VerifyContentEquals(before_write + appended_content, actual_content);
}

TEST_F(FileTapeTest, WriteInt) {
  const auto file_name = file_prefix_ / "file";
  std::vector<int64_t> written_numbers(1024, 0);
  std::generate(
      written_numbers.begin(),
      written_numbers.end(),
      [random = std::random_device()]() mutable {
        return random();
      }
  );
  FileTape<int64_t, true> under_test(config_, file_name);

  const auto written = under_test.WriteN(written_numbers);
  under_test.MoveToBegin();
  const auto actual_numbers = under_test.ReadN(SIZE_MAX);

  EXPECT_EQ(written_numbers.size(), written);
  VerifyContentEquals(written_numbers, actual_numbers);
}

TEST_F(FileTapeTest, ReadDelay) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "aa";
  CreateFileWithBinaryContent(file_name, content);
  constexpr auto read_duration = 500ms;
  const auto expected_duration = content.size() * read_duration;

  config_.SetReadDuration(read_duration);
  FileTape<char, false> under_test(config_, file_name);

  const auto actual_duration = Measure<seconds>([&under_test] {
    ReadAllFromTape(under_test);
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, ForwardMoveDelay) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "aa";
  CreateFileWithBinaryContent(file_name, content);
  constexpr auto move_duration = 500ms;
  const auto expected_duration = content.size() * move_duration;

  config_.SetMoveDuration(move_duration);
  FileTape<char, false> under_test(config_, file_name);

  const auto actual_duration = Measure<seconds>([&under_test] {
    while (under_test.MoveForward()) {
    }
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, BackwardMoveDelay) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "aa";
  CreateFileWithBinaryContent(file_name, content);
  constexpr auto move_duration = 500ms;
  const auto expected_duration = content.size() * move_duration;

  config_.SetMoveDuration(move_duration);
  FileTape<char, false> under_test(config_, file_name);

  under_test.MoveToEnd();
  const auto actual_duration = Measure<seconds>([&under_test] {
    while (under_test.MoveBackward()) {
    }
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, WriteDelay) {
  const auto file_name = file_prefix_ / "file";
  const std::string content = "aa";
  constexpr auto write_duration = 500ms;
  const auto expected_duration = content.size() * write_duration;

  config_.SetWriteDuration(write_duration);
  FileTape<char> under_test(config_, file_name);

  const auto actual_duration = Measure<seconds>([&content, &under_test] {
    under_test.WriteN({content.begin(), content.end()});
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, MoveToBeginDelay) {
  const auto file_name = file_prefix_ / "file";
  constexpr auto rewind_duration = 1s;
  const auto expected_duration = rewind_duration;

  config_.SetRewindDuration(rewind_duration);
  FileTape<char> under_test(config_, file_name);

  const auto actual_duration = Measure<seconds>([&under_test] {
    under_test.MoveToBegin();
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, MoveToEndDuration) {
  const auto file_name = file_prefix_ / "file";
  constexpr auto rewind_duration = 1s;
  const auto expected_duration = rewind_duration;

  config_.SetRewindDuration(rewind_duration);
  FileTape<char> under_test(config_, file_name);

  const auto actual_duration = Measure<seconds>([&under_test] {
    under_test.MoveToEnd();
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

}  // namespace sot::test
