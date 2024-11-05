#include "LexemAnalizer.hpp"

LexemList::~LexemList() {
  for(Lexem * lexem : *this) {
    delete lexem;
  }
}

LexemList LexemAnalyzer::parse(std::string text) {
  LexemList lexems;


  return lexems;
};
