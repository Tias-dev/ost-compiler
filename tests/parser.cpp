#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include "CharStream.hpp"
#include "combinators/Result.hpp"
#include "parsers/main.hpp"

namespace  {
#include <gtest/gtest.h>

class ParserTestSuite : public testing::Test {
	const static std::string ostProgram_;
	std::stringstream ss_;
	CharStream* stream_;
	public:
	ICharStream & stream;
	ParserTestSuite() : ss_(ostProgram_), stream_(new CharStream(ss_)), stream(*stream_) {};
	public:
	void SetUp() {
			ss_ = std::stringstream(ostProgram_);
			stream_ = new CharStream(ss_);
	}
	void TearDown() {
		delete stream_;
	}
};

const std::string ParserTestSuite::ostProgram_ =
"MT НОД;\n"
"	BEGIN\n"
"		ALPHABET: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9;\n"
"";

TEST_F(ParserTestSuite, mtParser) {
	auto res = cmb::mt->parse(stream);

	ASSERT_EQ(res->status(), cmb::ResultStatus::Success);

	std::string data = std::get<0>(res->data().value());
	ASSERT_EQ(data, "НОД");
}

} // namespace 
