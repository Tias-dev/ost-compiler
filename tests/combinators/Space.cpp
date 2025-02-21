#include "combinators/Space.hpp"
#include <gtest/gtest.h>
#include "CharStream.hpp"
#include "Combinator.hpp"
#include "combinators/String.hpp"

namespace  {
  TEST(SpaceTestSuite, SpaceConsume) {
    std::stringstream ss("some valueble\ttext\n");
    CharStream charstream(ss);
    cmb::String p1("some"), p2("valueble"), p3("text");
    cmb::Space space;

    auto res = space.parse(charstream);
    EXPECT_EQ(res->status, cmb::ResultStatus::Failure);

    p1.parse(charstream);

    res = space.parse(charstream);
    EXPECT_EQ(res->status, cmb::ResultStatus::Success);

    p2.parse(charstream);

    res = space.parse(charstream);
    EXPECT_EQ(res->status, cmb::ResultStatus::Success);

    p3.parse(charstream);

    res = space.parse(charstream);
    EXPECT_EQ(res->status, cmb::ResultStatus::Success);

    res = space.parse(charstream);
    EXPECT_EQ(res->status, cmb::ResultStatus::Failure);
  }
} // namespace 
