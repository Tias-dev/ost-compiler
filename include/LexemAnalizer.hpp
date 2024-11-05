#ifndef LEXEM_ANALYZER_HPP_
#define LEXEM_ANALYZER_HPP_

#include "Lexem.hpp"
#include <list>
#include <string>

class LexemList : public std::list<Lexem *> {
  public:
    ~LexemList(); // Deallocate 
};

class LexemAnalyzer {

public:
  LexemList parse(std::string textl);
};

#endif //! LEXEM_ANALYZER_HPP_
