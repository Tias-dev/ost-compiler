#include "CharStream.hpp"
#include "combinators/Result.hpp"
#include "combinators/String.hpp"
#include "combinators/utils.hpp"
#include <gtest/gtest.h>
#include <sstream>

TEST(OptionalParserTestSuite, ReallySuccess) {
	std::string s = "some valueable text", pattern = "some";
	std::istringstream ss(s);
	CharStream stream(ss);
	auto parser = cmb::opt(cmb::String::create(pattern));

	auto res = parser->parse(stream);

	ASSERT_EQ(res->status(), cmb::ResultStatus::Success);
	ASSERT_EQ(res->parsedLen(), pattern.size());
}

TEST(OptionalParserTestSuite, OptionalSuccess) {
	std::string s = "some valueable text", pattern = "valueable";
	std::istringstream ss(s);
	CharStream stream(ss);
	auto parser = cmb::opt(cmb::String::create(pattern));

	auto res = parser->parse(stream);

	ASSERT_EQ(res->status(), cmb::ResultStatus::Success);
	ASSERT_EQ(res->parsedLen(), 0);
}
