#include "combinators/Space.hpp"
#include <gtest/gtest.h>
#include "CharStream.hpp"
#include "combinators/Combinator.hpp"
#include "combinators/String.hpp"

namespace  {
  TEST(SpaceTestSuite, SpaceConsume) {
    std::stringstream ss("some valueble\ttext\n");
    CharStream charstream(ss);
		std::string words[] = {"some", "valueble", "text"};
    auto p1 = cmb::String::create(words[0]);
		auto p2 = cmb::String::create(words[1]);
		auto p3 = cmb::String::create(words[2]);
    auto space = cmb::Space::create();

    auto res = space->parse(charstream);
    EXPECT_EQ(res->status(), cmb::ResultStatus::Failure);

    p1->parse(charstream);

    res = space->parse(charstream);
    EXPECT_EQ(res->status(), cmb::ResultStatus::Success);

    p2->parse(charstream);

    res = space->parse(charstream);
    EXPECT_EQ(res->status(), cmb::ResultStatus::Success);

    p3->parse(charstream);

    res = space->parse(charstream);
    EXPECT_EQ(res->status(), cmb::ResultStatus::Success);

    res = space->parse(charstream);
    EXPECT_EQ(res->status(), cmb::ResultStatus::Failure);
  }
} // namespace 
