#ifndef LEXEM_HPP_
#define LEXEM_HPP_

#include "keywords.hpp"
#include "lexemType.hpp"
#include "operators.hpp"
#include <cstddef>
#include <ostream>
#include <string>

class Lexem {
  LexemType type_;

	std::pair<size_t, std::string> id_ = {0, ""};

  OperatorType opType_ = OperatorType::None;
  KeywordType kwType_ = KeywordType::None;

public:
  Lexem(OperatorType opType) : type_(LexemType::Operator), opType_(opType) {}
  Lexem(KeywordType kwType) : type_(LexemType::Keyword), kwType_(kwType) {}
  Lexem(size_t id, std::string value)
      : type_(LexemType::Id), id_(id, value) {}

  LexemType type() const { return type_; }
  OperatorType op() const { return opType_; }
  KeywordType keyword() const { return kwType_; }

  const std::pair<size_t, std::string>& Id() const { return id_; }
};

inline std::ostream &operator<<(std::ostream &os, const Lexem &lexem) {
  switch (lexem.type()) {
  case LexemType::None:
    os << "None";
    break;
  case LexemType::Keyword:
    os << to_string(lexem.keyword());
    break;
  case LexemType::Operator:
    os << to_string(lexem.op());
    break;
  case LexemType::Id:
    os << "{ "<<lexem.Id().first << ": " << lexem.Id().second << " }" ;
    break;
  default:
		os << "Undefined";
  }

	return os;
}

#endif // !LEXEM_HPP_
