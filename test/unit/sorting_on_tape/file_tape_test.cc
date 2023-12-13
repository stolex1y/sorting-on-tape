#include "file_tape.h"

#include <gtest/gtest.h>

#include "fake_configuration.h"
#include "file_utils.h"
#include "test_base.h"
#include "test_utils.h"

namespace sot::test {

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace files;

class FileTapeTest : public TestBase, public testing::Test {
 public:
  static constexpr std::string kDefaultTestContent = "Test content";

  path input_file_path_;

  FileTapeTest();

  template <bool Mutable = false>
  FileTape<char, Mutable> InitTapeWithContent(const std::string &content = kDefaultTestContent);
};

FileTapeTest::FileTapeTest() {
  TestBase::SetUp("FileTapeTest", testing::UnitTest::GetInstance()->current_test_info()->name());
  input_file_path_ = file_prefix_ / "file";
}

template <bool Mutable>
FileTape<char, Mutable> FileTapeTest::InitTapeWithContent(const std::string &content) {
  CreateFileWithBinaryContent(input_file_path_, content);
  return {config_, input_file_path_};
}

TEST_F(FileTapeTest, ReadFromMutableFileTape) {
  FileTape under_test = InitTapeWithContent<true>();

  const auto actual_content = ReadAllFromTapeAsString(under_test);

  VerifyContentEquals(kDefaultTestContent, actual_content);
}

TEST_F(FileTapeTest, ReadUntilTrue) {
  FileTape under_test = InitTapeWithContent();

  std::string actual_content;
  while (const auto ch = under_test.Read()) {
    actual_content += *ch;
  }

  VerifyContentEquals(kDefaultTestContent, actual_content);
  VerifyCursorAtTheEnd(under_test);
}

TEST_F(FileTapeTest, ReadFromEmptyTape) {
  const std::string content;
  FileTape under_test = InitTapeWithContent(content);

  const auto actual_content = ReadAllFromTapeAsString(under_test);

  VerifyContentEquals(content, actual_content);
  VerifyCursorAtTheEnd(under_test);
  VerifyCursorAtTheBeginning<char>(under_test, std::nullopt);
}

TEST_F(FileTapeTest, ReadPartOfConent) {
  constexpr auto part_size = kDefaultTestContent.size() / 2;
  FileTape under_test = InitTapeWithContent();

  const auto chars = under_test.ReadN(part_size);
  const std::string actual_content(chars.begin(), chars.end());

  VerifyContentEquals(kDefaultTestContent.substr(0, part_size), actual_content);
  EXPECT_NE(std::nullopt, under_test.Read())
      << "After reading part of the content, there is nothing left";
}

TEST_F(FileTapeTest, MoveToEnd) {
  FileTape under_test = InitTapeWithContent();

  under_test.MoveToEnd();

  VerifyCursorAtTheEnd(under_test);
}

TEST_F(FileTapeTest, MoveForwardToReadPartOfContent) {
  constexpr auto skip = kDefaultTestContent.size() / 2;
  FileTape under_test = InitTapeWithContent();

  for (size_t i = 0; i < skip; ++i) {
    under_test.MoveForward();
  }
  const auto actual_content = ReadAllFromTapeAsString(under_test);

  VerifyContentEquals(kDefaultTestContent.substr(skip), actual_content);
}

TEST_F(FileTapeTest, MoveBackwardFromBegin) {
  FileTape under_test = InitTapeWithContent();

  VerifyCursorAtTheBeginning<char>(under_test, kDefaultTestContent.front());
}

TEST_F(FileTapeTest, MoveBackwardFromEnd) {
  FileTape under_test = InitTapeWithContent();

  under_test.MoveToEnd();
  under_test.MoveBackward();
  const auto last_char = under_test.Read();

  EXPECT_EQ(kDefaultTestContent.back(), last_char)
      << "After moving back, there must be the last character";
}

TEST_F(FileTapeTest, MoveToBegin) {
  FileTape under_test = InitTapeWithContent();

  under_test.MoveToEnd();
  under_test.MoveToBegin();

  VerifyCursorAtTheBeginning<char>(under_test, kDefaultTestContent.front());
}

TEST_F(FileTapeTest, ReadFullContentTwice) {
  FileTape under_test = InitTapeWithContent();

  const auto first_content = ReadAllFromTapeAsString(under_test);
  under_test.MoveToBegin();
  const auto second_content = ReadAllFromTapeAsString(under_test);

  VerifyContentEquals(kDefaultTestContent, first_content);
  VerifyContentEquals(kDefaultTestContent, second_content);
}

TEST_F(FileTapeTest, WriteToImmutableFileTape) {
  const std::string new_content = "Updated. " + kDefaultTestContent + " Updated.";
  FileTape under_test = InitTapeWithContent<false>();

  const auto written = under_test.WriteN({new_content.begin(), new_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTapeAsString(under_test);

  EXPECT_EQ(0, written) << "Mustn't be written to immutable tape";
  VerifyContentEquals(kDefaultTestContent, actual_content);
}

TEST_F(FileTapeTest, WriteToMutableFileTape) {
  const std::string new_content = "Updated. " + kDefaultTestContent + " Updated.";
  FileTape under_test = InitTapeWithContent<true>();

  const auto written = under_test.WriteN({new_content.begin(), new_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTapeAsString(under_test);

  EXPECT_EQ(new_content.size(), written);
  VerifyContentEquals(new_content, actual_content);
}

TEST_F(FileTapeTest, WriteToMutableFileTapeUsingIterators) {
  const std::string new_content = "Updated. " + kDefaultTestContent + " Updated.";
  const std::vector new_content_chars(new_content.begin(), new_content.end());
  FileTape under_test = InitTapeWithContent<true>();

  const auto last_written = under_test.WriteN(new_content_chars.begin(), new_content_chars.end());
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTapeAsString(under_test);

  const auto written = std::distance(new_content_chars.begin(), last_written);
  EXPECT_EQ(new_content.size(), written);
  VerifyContentEquals(new_content, actual_content);
}

TEST_F(FileTapeTest, AppendToMutableFileTape) {
  const std::string appended_content = ". Appended content";
  FileTape under_test = InitTapeWithContent<true>();

  under_test.MoveToEnd();
  const auto written = under_test.WriteN({appended_content.begin(), appended_content.end()});
  under_test.MoveToBegin();
  const auto actual_content = ReadAllFromTapeAsString(under_test);

  EXPECT_EQ(appended_content.size(), written);
  VerifyContentEquals(kDefaultTestContent + appended_content, actual_content);
}

TEST_F(FileTapeTest, WriteInt) {
  using Value = std::int64_t;
  const auto written_numbers = GenerateRandomArray<Value>(1000);
  FileTape<Value, true> under_test(config_, input_file_path_);

  const auto written = under_test.WriteN(written_numbers);
  under_test.MoveToBegin();
  const auto actual_numbers = under_test.ReadN(SIZE_MAX);

  EXPECT_EQ(written_numbers.size(), written);
  VerifyContentEquals(written_numbers, actual_numbers);
}

TEST_F(FileTapeTest, ReadDelay) {
  constexpr auto read_duration = 500ms;
  config_.SetReadDuration(read_duration);

  const std::string content = "aa";
  const auto expected_duration = content.size() * read_duration;
  FileTape under_test = InitTapeWithContent(content);

  const auto actual_duration = Measure<seconds>([&under_test] {
    ReadAllFromTapeAsString(under_test);
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, ForwardMoveDelay) {
  constexpr auto move_duration = 500ms;
  config_.SetMoveDuration(move_duration);

  const std::string content = "aa";
  const auto expected_duration = content.size() * move_duration;
  FileTape under_test = InitTapeWithContent(content);

  const auto actual_duration = Measure<seconds>([&under_test] {
    while (under_test.MoveForward()) {
    }
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, BackwardMoveDelay) {
  constexpr auto move_duration = 500ms;
  config_.SetMoveDuration(move_duration);

  const std::string content = "aa";
  const auto expected_duration = content.size() * move_duration;
  FileTape under_test = InitTapeWithContent(content);

  under_test.MoveToEnd();
  const auto actual_duration = Measure<seconds>([&under_test] {
    while (under_test.MoveBackward()) {
    }
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, WriteDelay) {
  constexpr auto write_duration = 500ms;
  config_.SetWriteDuration(write_duration);

  const std::string content = "aa";
  const auto expected_duration = content.size() * write_duration;
  FileTape under_test = InitTapeWithContent<true>(content);

  const auto actual_duration = Measure<seconds>([&content, &under_test] {
    under_test.WriteN({content.begin(), content.end()});
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, MoveToBeginDelay) {
  constexpr auto rewind_duration = 1s;
  config_.SetRewindDuration(rewind_duration);

  const auto expected_duration = rewind_duration;
  FileTape under_test = InitTapeWithContent();

  const auto actual_duration = Measure<seconds>([&under_test] {
    under_test.MoveToBegin();
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

TEST_F(FileTapeTest, MoveToEndDuration) {
  constexpr auto rewind_duration = 1s;
  config_.SetRewindDuration(rewind_duration);

  const auto expected_duration = rewind_duration;
  FileTape under_test = InitTapeWithContent();

  const auto actual_duration = Measure<seconds>([&under_test] {
    under_test.MoveToEnd();
  });

  EXPECT_EQ(expected_duration, actual_duration);
}

}  // namespace sot::test
