#include "Token.hpp"
#include "exception.hpp"
#include <cstdio>
#include <string>
#include <utility>

namespace token {
Token::Token(size_t begin, size_t end, Type type)
    : id_(currentId++), begin_(begin), end_(end), type_(type) {
  if (begin >= end)
    throw error::BordersError(begin, end);
}
} // namespace token
namespace impl {
std::pair<OpTrie, KwTrie> initTries() {
  KwTrie kw;
  kw.add("MT", token::KwType::MT);
  kw.add("BEGIN", token::KwType::BEGIN);
  kw.add("END", token::KwType::END);
  kw.add("ALPHABET", token::KwType::ALPHABET);
  kw.add("IF", token::KwType::IF);
  kw.add("FI", token::KwType::FI);
  kw.add("DO", token::KwType::DO);
  kw.add("OD", token::KwType::OD);
  kw.add("LIB", token::KwType::LIB);

  OpTrie op;
  op.add(";", token::OpType::TERMINATOR);
  op.add(",", token::OpType::COMA);
  op.add(":", token::OpType::SEMICOLON);
  op.add("a", token::OpType::SET_LETTER);
  op.add("(", token::OpType::LEFT_BRACKET);
  op.add(")", token::OpType::RIGHT_BRACKET);
  op.add("_", token::OpType::LAMBDA);
  op.add("**", token::OpType::POW);
  op.add("?", token::OpType::QUESTION);
  op.add("!=", token::OpType::NOT_EQUAL);

  return {op, kw};
}

const std::pair<OpTrie::bimap_type, KwTrie::bimap_type> &getBimaps() {
  static const auto [op, kw] = initTries();
  static const std::pair<OpTrie::bimap_type, KwTrie::bimap_type> bimaps = {
      op.bimap(), kw.bimap()};

  return bimaps;
}

static const auto &[opBimap, kwBimap] = getBimaps();
} // namespace impl

std::string token::Keyword::toString() const {
  return typeToString() + ": " + impl::kwBimap.toString(kwtype_);
}

std::string token::Operation::toString() const {
  return typeToString() + ": " + impl::opBimap.toString(optype_);
}
