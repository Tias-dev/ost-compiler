#include "CharStream.hpp"
#include "combinators/Combinator.hpp"
#include "combinators/utils.hpp"
#include "combinators/String.hpp"
#include "combinators/Space.hpp"
#include <gtest/gtest.h>
#include <iterator>
#include <src/gtest-internal-inl.h>

namespace {

using namespace cmb;
class GroupingTest : public testing::Test {
public:
  std::string str;
  std::stringstream ss;
  std::vector<std::string> words;
  std::vector<ptr<>> wordsParsers{String::create(words[0]),
                                  String::create(words[1]),
                                  String::create(words[2])};
	ptr<char> space = Space::create();

  GroupingTest()
      : str("some valueable text"), ss(str),
        words({"some", "valueable", "text"}) {}

  CharStream charstream{ss};
  void SetUp() {}
  void TearDown() {}
};

TEST_F(GroupingTest, redirectLeft) {
	std::string s("some");
	auto charParser = AnyOf::create(s);
	auto parser = charParser << charParser;
	auto res = parser->parse(charstream);

	ASSERT_EQ(res->status(), ResultStatus::Success);
	ASSERT_EQ(res->parsedLen(), 2);
	ASSERT_EQ(std::get<0>(res->data().value()), 's');
}

TEST_F(GroupingTest, redirectRight) {
	std::string s("some");
	auto charParser = AnyOf::create(s);
	auto parser = charParser >> charParser;
	auto res = parser->parse(charstream);

	ASSERT_EQ(res->status(), ResultStatus::Success);
	ASSERT_EQ(res->parsedLen(), 2);
	ASSERT_EQ(std::get<0>(res->data().value()), 'o');
}

TEST_F(GroupingTest, LogicalAnd) {
	auto parser = wordsParsers[0] && space && wordsParsers[1];
	auto res = parser->parse(charstream);

	ASSERT_EQ(res->status(), ResultStatus::Success);
	ASSERT_EQ(res->parsedLen(), words[0].size() + 1 + words[1].size());
}

TEST_F(GroupingTest, LogicalOr) {
	auto parser = wordsParsers[1] || wordsParsers[0];
	auto res = parser->parse(charstream);

	ASSERT_EQ(res->status(), ResultStatus::Success);
	ASSERT_EQ(res->parsedLen(), words[0].size());
}

TEST_F(GroupingTest, testMany) {
	std::string s = "some";
	auto charParser = AnyOf::create(s); 
	auto parser = many(charParser);
	auto res = parser->parse(charstream);

	ASSERT_EQ(res->status(), ResultStatus::Success);
	ASSERT_EQ(res->parsedLen(), words[0].size());

	auto iter = std::begin(words[0]);
	for(auto elem : std::get<0>(res->data().value())) {
		ASSERT_EQ(elem->status(), ResultStatus::Success);
		char c = std::get<0>(elem->data().value());

		ASSERT_EQ(c, *iter);
		++iter;
	}
}
} // namespace
