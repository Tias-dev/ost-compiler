#include "CharStream.hpp"
#include "Lexem.hpp"
#include "LexemAnalizer.hpp"
#include "keywords.hpp"
#include "operators.hpp"
#include <cctype>
#include <map>

LexemList::~LexemList() {
  for (Lexem *lexem : *this) {
    delete lexem;
  }
}

static size_t currentId = 0;

std::list<Lexem *> parseBuffer(const std::string &buffer, std::map<std::string, size_t> & savedIds) {
  auto kw = getKeywordType(buffer);
  if (kw != KeywordType::None)
    return {new Lexem(kw)};

  auto op = getOperatorType(buffer);
  if (op != OperatorType::None)
    return {new Lexem(op)};

  std::list<Lexem *> result;
  for (size_t i = 0; i < buffer.size(); ++i) {
    std::string substr = buffer.substr(i, std::string::npos);
    if (isOperator(substr)) {
      auto beginSubStr = buffer.substr(0, i);
      auto parsedBegin = parseBuffer(beginSubStr, savedIds);
      for (auto &lexem : parsedBegin)
        result.push_back(lexem);

      result.push_back(new Lexem{getOperatorType(substr)});
      return result;
    }
  }
	if(savedIds.find(buffer) == savedIds.end()) 
		savedIds[buffer] = currentId++; 
	
	return {new Lexem(savedIds[buffer], buffer)};
}

bool isEndsByOperator(const std::string &buffer) {
  for (size_t i = 0; i < buffer.size(); ++i) {
    std::string substr = buffer.substr(i, std::string::npos);
    if (isOperator(substr))
      return true;
  }
  return false;
}

void processLetter(const char &c, std::string &buffer, LexemList &lexems, std::map<std::string, size_t> & savedIds) {
  if (isspace(c) || c == EOF) {
    if (buffer.empty())
      return;
    auto parsedBuffer = parseBuffer(buffer, savedIds);
    for (auto &lexem : parsedBuffer)
      lexems.push_back(lexem);
    buffer = "";
    return;
  }

  if (isEndsByOperator(buffer)) {
    auto parsedBuffer = parseBuffer(buffer, savedIds);
    for (auto &lexem : parsedBuffer)
      lexems.push_back(lexem);
    buffer = "";
  }

  buffer += c;
}

LexemList LexemAnalyzer::parse(ICharStream &stream) {
	std::map<std::string, size_t> savedIds;
  LexemList lexems;
  char c;
  std::string buffer = "";
  while (stream >> c, c != EOF) {
    processLetter(c, buffer, lexems, savedIds);
  }
  processLetter(c, buffer, lexems, savedIds);

  return lexems;
};
