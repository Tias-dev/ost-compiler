#ifndef KEYWORDS_HPP_
#define KEYWORDS_HPP_

#include <string>
enum class KeywordType {
  None,
  BEGIN,
  END,
  DO,
  OD,
  IF,
  FI,
  MT,
  ALPHABET,
  LIB,
};

bool isKeyword(const std::string &);
KeywordType getKeywordType(const std::string &);
std::string to_string(KeywordType type);

#endif // !KEYWORDS_HPP_
