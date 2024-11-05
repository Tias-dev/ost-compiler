#ifndef LEXEM_HPP_
#define LEXEM_HPP_

#include "lexemType.hpp"
class Lexem {
  LexemType type_;
public:
  Lexem(LexemType type) : type_(type) {}
};

#endif // !LEXEM_HPP_
