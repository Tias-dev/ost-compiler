#ifndef TOKEN_HPP_
#define TOKEN_HPP_

#include "utils.hpp"
#include <cstddef>
#include <stdexcept>

namespace token {
enum class Type { NAME, KEYWORD, OPERATOR };

class Token {
  static size_t currentId;
  size_t id_;
  size_t begin_;
  size_t end_;
  Type type_;

protected:
  Token(size_t begin, size_t end, Type type)
      : id_(currentId++), begin_(begin), end_(end), type_(type) {
    if (begin >= end)
      log::exception<std::invalid_argument>()
          << "begin(" << begin << ") of token must be less then end(" << end
          << ")" << std::endl;
  }
};

inline size_t Token::currentId = 0;

class Name : public Token {
  std::string name_;

public:
  Name(std::string name, size_t begin, size_t end)
      : Token(begin, end, Type::NAME), name_(name) {}
};

enum class KwType { MT, BEGIN, END, ALPHABET, IF, FI, DO, OD };

class Keyword : public Token {
  KwType kwtype_;

public:
  Keyword(KwType type, size_t begin, size_t end)
      : Token(begin, end, Type::NAME), kwtype_(type) {}
};

enum class OpType {
  TERMINATOR,
  COMA,
  SEMICOLON,
  SET_LETTER,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  LAMBDA,
  POW,
  QUESTION,
  NOT_EQUAL
};

class Operation : public Token {
  OpType optype_;

public:
  Operation(OpType type, size_t begin, size_t end)
      : Token(begin, end, Type::NAME), optype_(type) {}
};
} // namespace token

#endif // !TOKEN_HPP_
