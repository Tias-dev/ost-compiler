#include "CharStream.hpp"
#include "LexemAnalizer.hpp"

LexemList::~LexemList() {
  for(Lexem * lexem : *this) {
    delete lexem;
  }
}

LexemList LexemAnalyzer::parse(const CharStream & stream) {
  LexemList lexems;


  return lexems;
};
