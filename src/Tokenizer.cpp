#include "Tokenizer.hpp"
#include "CharStream.hpp"
#include "Token.hpp"
#include "exception.hpp"
#include <cctype>
#include <utility>
#include <variant>

bool isSpace(int c) { return isspace(c) || (c == '"'); }

/**
 * @brief Read token from stream. Assuming that no spaces before i.e. token
 * begin at current position
 *
 * @param stream -- ICharStream realization
 *
 * @return one of tokens in union type
 */
token::token_union read_token(ICharStream &stream) {
  static auto [op, kw] = impl::initTries();
  auto opPoint = op.begin();
  auto kwPoint = kw.begin();
  size_t begin = stream.position();

  char c;
  stream >> c;
  if (opPoint->canGoTo(c)) {
    while (opPoint->canGoTo(c) && !opPoint->isTerm()) {
      opPoint->goTo(c);
      if (stream.eof())
        break;
      stream >> c;
    }
    stream << c;

    if (!opPoint->isTerm())
      throw error::UndefinedOperatorError(stream.position());

    auto opType = opPoint->get();
    return {token::Operation(opType, begin, stream.position())};
  }
  std::string buffer = "";
  while (!stream.eof() && !isSpace(c) && !opPoint->canGoTo(c)) {
    buffer.push_back(c);
    stream >> c;
  }

  if (!stream.eof())
    stream << c;

  size_t i;
  for (i = 0; i < buffer.size(); ++i)
    if (kwPoint->canGoTo(buffer[i]))
      kwPoint->goTo(buffer[i]);
    else
      break;

  if (i == buffer.size() && kwPoint->isTerm())
    return {token::Keyword(kwPoint->get(), begin, stream.position())};
  return {token::Name(buffer, begin, stream.position())};
}

token::tokens_list token::Tokenizer::parse(ICharStream &stream) {
  char c;
  token::tokens_list result;
  stream >> c;
  while (!stream.eof()) {
    if (c == '"') {
      size_t commentBegin = stream.position();
      do {
        stream >> c;
      } while (!stream.eof() && (c != '"'));
      if (c != '"' || stream.eof())
        throw error::CommentNotClosedError(commentBegin);
      continue;
    }

    if (!isspace(c)) {
      stream << c;
      result.push_back(read_token(stream));
    }
    stream >> c;
  }

  return result;
}

namespace token {
bool token_union::operator==(const KwType type) const {
  return std::visit(
      overloads{[](const Name &) { return false; },
                [type](const Keyword &token) { return type == token.type(); },
                [](const Operation &) { return false; }},
      data_);
}

bool token_union::operator==(const OpType type) const {
  return std::visit(overloads{[](const Name &token) { return false; },
                              [](const Keyword &token) { return false; },
                              [type](const Operation &token) {
                                return type == token.type();
                              }},
                    data_);
}

bool token_union::operator!=(const KwType type) const {
  return !(*this == type);
}

bool token_union::operator!=(const OpType type) const {
  return !(*this == type);
}

std::string token_union::toString() const {
  return std::visit([](const auto &token) { return token.toString(); }, data_);
}

std::string token_union::typeToString() const {
  return std::visit([](const auto &token) { return token.typeToString(); },
                    data_);
}

bool token_union::isName() const {
  return std::visit(overloads{[](const Name &) { return true; },
                              [](const Keyword &) { return false; },
                              [](const Operation &) { return false; }},
                    data_);
}

std::string token_union::getName() const {
  return std::visit(
      overloads{[](const Name &token) { return token.name(); },
                [](const Keyword &token) {
                  throw error::ExpectedMismatchError(
                      token.begin(), "valid mt name", token.toString());
                  return std::string();
                },
                [](const Operation &token) {
                  throw error::ExpectedMismatchError(
                      token.begin(), "valid mt name", token.toString());
                  return std::string();
                }},
      data_);
}

size_t token_union::begin() const {
  return std::visit([](const auto &token) { return token.begin(); }, data_);
}

size_t token_union::end() const {
  return std::visit([](const auto &token) { return token.end(); }, data_);
}

bool operator==(const KwType type, const token_union &token) {
  return token == type;
}

bool operator==(const OpType type, const token_union &token) {
  return token == type;
}

bool operator!=(const KwType type, const token_union &token) {
  return token != type;
}

bool operator!=(const OpType type, const token_union &token) {
  return token != type;
}

void tokens_list::Session::commit() {
  while (!cache_.empty())
    cache_.pop();
}
void tokens_list::Session::rollback() {
  while (!cache_.empty()) {
    root_->push_front(cache_.top());
    cache_.pop();
  }
}

void tokens_list::Session::popFront() {
  cache_.push(*std::begin(*root_));
  root_->pop_front();
}

token_union tokens_list::Session::popFrontAndReturn() {
  if (root_->empty())
    throw error::UnexpectedFileEnd();

  cache_.push(*std::begin(*root_));
  token_union token = *std::begin(*root_);
  root_->pop_front();
  return token;
}

tokens_list::Session::~Session() { rollback(); }
} // namespace token
