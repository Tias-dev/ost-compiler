#include "CharStream.hpp"
#include "LexemAnalizer.hpp"
#include "keywords.hpp"
#include "operators.hpp"
#include <cctype>

LexemList::~LexemList() {
  for(Lexem * lexem : *this) {
    delete lexem;
  }
}

static size_t currentId = 0;

Lexem *parseBuffer(const std::string &buffer) {
	auto kw = getKeywordType(buffer);
	if(kw != KeywordType::None) 
		return new Lexem(kw);

	auto op = getOperatorType(buffer);
	if(op != OperatorType::None)
		return new Lexem(op);

	return new Lexem(currentId++, buffer);
}

void processLetter(const char &c, std::string & buffer, LexemList & lexems) {
		if(isspace(c) || c == EOF) {
			if(buffer.empty())
				return;
			lexems.push_back(parseBuffer(buffer));
			buffer = "";
			return;
		}

		buffer += c;
}

LexemList LexemAnalyzer::parse(ICharStream& stream) {
  LexemList lexems;
	char c;
	std::string buffer = "";
	while(stream >> c, c != EOF) {
		if(isOperator(buffer)) {
			lexems.push_back(parseBuffer(buffer));
			buffer = "";
			continue;
		}
		processLetter(c, buffer, lexems);
	}
	processLetter(c, buffer, lexems);
	
  return lexems;
};
