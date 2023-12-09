#include "file_tape.h"

#include <gtest/gtest.h>

#include <random>

#include "fake_configuration.h"
#include "file_utils.h"

namespace sot::test {

using namespace files;

class FileTapeTest : public testing::Test {
 public:
  FileTapeTest();

  FakeConfiguration config_;
  std::string test_name_ = testing::UnitTest::GetInstance()->current_test_info()->name();
  std::string file_prefix_ = "FileTapeTest/" + test_name_ + "/";
};

FileTapeTest::FileTapeTest() {
  CreateDir(file_prefix_);
}

template <bool Mutable>
std::string ReadAll(FileTape<char, Mutable> &tape) {
  const auto chars = tape.ReadN(std::string::npos);
  return {chars.begin(), chars.end()};
}

TEST_F(FileTapeTest, ReadFromMutableFileTape) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content\nTest content";
  CreateFileWithContent(file_name, content);
  FileTape<char> under_test(config_, file_name);

  const auto actual_content = ReadAll(under_test);

  EXPECT_EQ(content, actual_content) << "Content from the mutable file tape is different";
}

TEST_F(FileTapeTest, ReadUntilTrue) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content";
  CreateFileWithContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  std::string actual_content;
  while (const auto ch = under_test.Read()) {
    actual_content += *ch;
  }

  EXPECT_EQ(content, actual_content) << "Content from the file tape is different";
  EXPECT_EQ(false, under_test.MoveForward()) << "Mustn't move forward from the end";
}

TEST_F(FileTapeTest, ReadFromEmptyTape) {
  const auto file_name = file_prefix_ + "file";
  const std::string content;
  CreateFileWithContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  const auto actual_content = ReadAll(under_test);

  EXPECT_EQ(content, actual_content) << "Content from the file tape is different";
  EXPECT_EQ(false, under_test.MoveForward()) << "Mustn't move forward from the end";
  EXPECT_EQ(false, under_test.MoveBackward()) << "Mustn't move backward from the begin";
}

TEST_F(FileTapeTest, ReadPartOfConent) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content";
  constexpr auto part_size = 4;
  CreateFileWithContent(file_name, content);
  FileTape<char> under_test(config_, file_name);

  const auto chars = under_test.ReadN(part_size);
  const std::string actual_content(chars.begin(), chars.end());

  EXPECT_EQ(content.substr(0, part_size), actual_content)
      << "Content from the file tape is different";
  EXPECT_NE(std::nullopt, under_test.Read())
      << "After reading part of the content, there is nothing left";
}

TEST_F(FileTapeTest, MoveToEnd) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content";
  CreateFileWithContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  under_test.MoveToEnd();

  EXPECT_EQ(false, under_test.MoveForward()) << "Mustn't move forward from the end";
  EXPECT_EQ(std::nullopt, under_test.Read()) << "The cursor is not at the end";
}

TEST_F(FileTapeTest, MoveBackwardFromBegin) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content";
  CreateFileWithContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  EXPECT_EQ(false, under_test.MoveBackward()) << "Mustn't move backward from the begin";
  EXPECT_EQ(content.front(), under_test.Read()) << "The cursor is not at the begin";
}

TEST_F(FileTapeTest, MoveBackwardFromEnd) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content";
  CreateFileWithContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  under_test.MoveToEnd();
  under_test.MoveBackward();
  const auto last_char = under_test.Read();

  EXPECT_EQ(content.back(), last_char) << "After moving back, there must be the last character";
}

TEST_F(FileTapeTest, MoveToBegin) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content";
  CreateFileWithContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  under_test.MoveToEnd();
  under_test.MoveToBegin();

  EXPECT_EQ(false, under_test.MoveBackward()) << "Mustn't move backward from the begin";
  EXPECT_EQ(content.front(), under_test.Read()) << "The cursor is not at the begin";
}

TEST_F(FileTapeTest, ReadFullContentTwice) {
  const auto file_name = file_prefix_ + "file";
  const std::string content = "Test content";
  CreateFileWithContent(file_name, content);
  FileTape<char, false> under_test(config_, file_name);

  const auto first_content = ReadAll(under_test);
  under_test.MoveToBegin();
  const auto second_content = ReadAll(under_test);

  EXPECT_EQ(content, first_content) << "Content from the first reading is different";
  EXPECT_EQ(content, second_content) << "Content from the second reading is different";
}

TEST_F(FileTapeTest, WriteToImmutableFileTape) {
  const auto file_name = file_prefix_ + "file";
  const std::string before_write = "Test content";
  const std::string new_content = "Updated message";
  CreateFileWithContent(file_name, before_write);
  FileTape<char, false> under_test(config_, file_name);

  const auto written = under_test.WriteN({new_content.begin(), new_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAll(under_test);

  EXPECT_EQ(0, written) << "Mustn't be written to immutable tape";
  EXPECT_EQ(before_write, actual_content) << "The original message must not be overwritten";
}

TEST_F(FileTapeTest, WriteToMutableFileTape) {
  const auto file_name = file_prefix_ + "file";
  const std::string before_write = "Test content";
  const std::string new_content = "Updated message";
  CreateFileWithContent(file_name, before_write);
  FileTape<char, true> under_test(config_, file_name);

  const auto written = under_test.WriteN({new_content.begin(), new_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAll(under_test);

  EXPECT_EQ(new_content.size(), written);
  EXPECT_EQ(new_content, actual_content) << "The original message must be overwritten";
}

TEST_F(FileTapeTest, AppendToMutableFileTape) {
  const auto file_name = file_prefix_ + "file";
  const std::string before_write = "Test content";
  const std::string appended_content = ". Appended content";
  CreateFileWithContent(file_name, before_write);
  FileTape<char, true> under_test(config_, file_name);

  under_test.MoveToEnd();
  const auto written = under_test.WriteN({appended_content.begin(), appended_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAll(under_test);

  EXPECT_EQ(appended_content.size(), written);
  EXPECT_EQ(before_write + appended_content, actual_content)
      << "The original message must be completed";
}

TEST_F(FileTapeTest, WriteInt) {
  const auto file_name = file_prefix_ + "file";
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
  EXPECT_EQ(written_numbers, actual_numbers);
}

}  // namespace sot::test
