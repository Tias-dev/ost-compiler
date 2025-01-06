#ifndef LEXEM_ANALYZER_HPP_
#define LEXEM_ANALYZER_HPP_

#include "CharStream.hpp"
#include "Lexem.hpp"
#include <list>

class LexemList : public std::list<Lexem *> {
  public:
    virtual ~LexemList(); // Deallocate 
};

class LexemAnalyzer {
public:
  LexemList parse(const CharStream & stream);
};

#endif //! LEXEM_ANALYZER_HPP_
