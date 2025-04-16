#ifndef TOKEN_HPP_
#define TOKEN_HPP_

#include "exception.hpp"
#include "trie.hpp"
#include <cstddef>
#include <string>

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
      throw error::BordersError(begin, end);
  }

public:
  virtual std::string toString() const = 0;
  virtual size_t begin() const { return begin_; }
  virtual size_t end() const { return end_; }
};

inline size_t Token::currentId = 0;

class Name : public Token {
  std::string name_;

public:
  Name(std::string name, size_t begin, size_t end)
      : Token(begin, end, Type::NAME), name_(name) {}

  std::string toString() const override { return typeToString() + ": " + name_; }
	static std::string typeToString() { return "Name"; }
};

enum class KwType { MT, BEGIN, END, ALPHABET, IF, FI, DO, OD };

class Keyword : public Token {
  KwType kwtype_;

public:
  Keyword(KwType type, size_t begin, size_t end)
      : Token(begin, end, Type::NAME), kwtype_(type) {}

  std::string toString() const override;
	static std::string typeToString() { return "Keyword"; }
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

  std::string toString() const override;
	static std::string typeToString(){ return "Operation"; }
};

} // namespace token

namespace impl {
using OpTrie = impl::Trie<token::OpType>;
using KwTrie = impl::Trie<token::KwType>;
std::pair<OpTrie, KwTrie> initTries();
} // namespace impl

#endif // !TOKEN_HPP_
