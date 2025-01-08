#include "CharStream.hpp"
#include "LexemAnalizer.hpp"
#include <memory>

LexemList::~LexemList() {
  for(Lexem * lexem : *this) {
    delete lexem;
  }
}

LexemList LexemAnalyzer::parse(std::shared_ptr<ICharStream> stream) {
  LexemList lexems;
  // TODO
  return lexems;
};
