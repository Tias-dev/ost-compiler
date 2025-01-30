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

  cmb::Result<> result = parser.parse(charstream);
  cmb::Result<> result2 = parser2.parse(charstream);

  ASSERT_EQ(result.status, cmb::ResultStatus::Success);
  ASSERT_EQ(result2.status, cmb::ResultStatus::Success);
}

TEST(CombinatorsTestSuite, StringFailed) {
  std::stringstream ss("some valueble text");
  CharStream charstream(ss);
  cmb::String parser("some");
  cmb::String parser2(" valueble");

  cmb::Result<> result = parser2.parse(charstream);
  cmb::Result<> result2 = parser.parse(charstream);

  ASSERT_EQ(result.status, cmb::ResultStatus::Failure);
  ASSERT_EQ(result2.status, cmb::ResultStatus::Success);
}
