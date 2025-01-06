#ifndef LEXEM_HPP_
#define LEXEM_HPP_

#include "lexemType.hpp"
#include <cstddef>

class Lexem {
  size_t id_;
  LexemType type_;
public:
  Lexem(LexemType type, size_t id) : type_(type), id_(id) {}
};

#endif // !LEXEM_HPP_
