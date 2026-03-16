#ifndef TOKEN_HPP_
#define TOKEN_HPP_

#include "FilePosition.hpp"
#include "trie.hpp"
#include <cstddef>
#include <string>

namespace token {
enum class Type { NAME, KEYWORD, OPERATOR };

class Token {
  static size_t currentId;
  size_t id_;
	FilePosition begin_, end_;
  Type type_;

protected:
  Token(const FilePosition & begin, const FilePosition & end, Type type);

public:
  virtual std::string toString() const = 0;
  virtual const FilePosition & begin() const { return begin_; }
  virtual const FilePosition & end() const { return end_; }
};

inline size_t Token::currentId = 0;

class Name : public Token {
  std::string name_;

public:
  Name(std::string name, const FilePosition & begin, const FilePosition & end)
      : Token(begin, end, Type::NAME), name_(name) {}

  std::string toString() const override {
    return typeToString() + ": " + name_;
  }
  static std::string typeToString() { return "Name"; }

  const std::string &name() const { return name_; }
};

enum class KwType {
  MT,
  BEGIN,
  END,
  ALPHABET,
  IF,
	ELSE,
  FI,
  DO,
  OD,
  LIB,
  LAMBDA,
  SET_LETTER
};

class Keyword : public Token {
  KwType kwtype_;

public:
  Keyword(KwType type, const FilePosition & begin, const FilePosition & end)
      : Token(begin, end, Type::NAME), kwtype_(type) {}

  std::string toString() const override;
  static std::string typeToString() { return "Keyword"; }
  KwType type() const { return kwtype_; }
};

enum class OpType {
  TERMINATOR,
  COMA,
  SEMICOLON,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  POW,
  QUESTION,
  NOT_EQUAL,
	BRANCH_SEPARATOR
};

class Operation : public Token {
  OpType optype_;

public:
  Operation(OpType type, const FilePosition & begin, const FilePosition & end)
      : Token(begin, end, Type::NAME), optype_(type) {}

  std::string toString() const override;
  OpType type() const { return optype_; }

  static std::string typeToString() { return "Operation"; }
};

} // namespace token

namespace impl {
using OpTrie = impl::Trie<token::OpType>;
using KwTrie = impl::Trie<token::KwType>;
std::pair<OpTrie, KwTrie> initTries();
const std::pair<OpTrie::bimap_type, KwTrie::bimap_type> &getBimaps();
} // namespace impl

#endif // !TOKEN_HPP_
