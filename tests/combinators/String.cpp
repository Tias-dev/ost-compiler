#include "combinators/String.hpp"
#include "CharStream.hpp"
#include "Combinator.hpp"
#include <gtest/gtest.h>
#include <sstream>

TEST(CombinatorsTestSuite, StringSucceed) {
  std::stringstream ss("some valueble text");
  CharStream charstream(ss);
  cmb::String parser("some");
  cmb::String parser2(" valueble");

  auto result = parser.parse(charstream);
  auto result2 = parser2.parse(charstream);

  ASSERT_EQ(result->status, cmb::ResultStatus::Success);
  ASSERT_EQ(result2->status, cmb::ResultStatus::Success);
}

TEST(CombinatorsTestSuite, StringFailed) {
  std::stringstream ss("some valueble text");
  CharStream charstream(ss);
  cmb::String parser("some");
  cmb::String parser2(" valueble");

  auto result = parser2.parse(charstream);
  auto result2 = parser.parse(charstream);

  ASSERT_EQ(result->status, cmb::ResultStatus::Failure);
  ASSERT_EQ(result2->status, cmb::ResultStatus::Success);
}
