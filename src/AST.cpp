#include "AST.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <iterator>
#include <optional>
#include <variant>

using namespace ast;

ast::Name::Name(token::tokens_list &tokens, std::string name)
    : NodeBase(ExprType::NAME), name_(name) {
  if (namesTable_.contains(name))
    id_ = namesTable_[name];
  else {
    id_ = counterId++;
    namesTable_.add(name_, id_);
  }

  init(tokens);
}

std::optional<size_t> ast::Name::getNameId(const std::string &name) {
  if (namesTable_.contains(name))
    return namesTable_[name];
  return std::nullopt;
}

std::optional<std::string> ast::Name::getNameById(const size_t id) {
  if (namesTable_.contains(id))
    return namesTable_[id];
  return std::nullopt;
}

MT::MT(token::tokens_list &tokens) : NodeBase(ExprType::MT) { init(tokens); }

Paired::Paired(token::tokens_list &tokens, token::KwType first,
               token::KwType second, bool forceTerminator)
    : NodeBase(ExprType::PAIRED), first_(first), second_(second),
      forceTerminator_(forceTerminator) {
  init(tokens);
};

Alphabet::Alphabet(token::tokens_list &tokens) : NodeBase(ExprType::ALPHABET) {
  init(tokens);
}

// -----------------------
// | init overloads next |
// -----------------------

template <class... Ts> struct overloads : Ts... {
  using Ts::operator()...;
};

void throwMismatch(const std::string &expected, token::token_union &given) {
  std::visit(
      [&expected](const auto &token) {
        throw error::ExpectedMismatchError(token.begin(), expected,
                                           token.toString());
      },
      given);
}

bool isName(token::token_union & token) {
	return std::visit(overloads {
			[](const token::Name&) {return true;},
			[](const auto & token) {return false;}
			}, token);
}

std::string getName(token::token_union & token) {
	return std::visit(overloads {
			[](const token::Name& token) {return token.toString();},
			[&token](const auto & token_) {throwMismatch("Name", token); return std::string();}
			}, token);
}

const auto &[opBimap, kwBimap] = impl::getBimaps();

void Name::init(token::tokens_list &tokens) {
	auto session = tokens.session();
  std::string value = getName(*std::begin(tokens));
  session.popFront();
  name_ = value;
	session.commit();
}

static const auto isKwVisitor = [](token::KwType type) {
  return overloads{
      [](token::Name &token) { return false; },
      [type](token::Keyword &token) { return token.type() == type; },
      [](token::Operation &token) { return false; }};
};
static const auto isOpVisitor = [](token::OpType type) {
  return overloads{
      [](token::Name &token) { return false; },
      [](token::Keyword &token) { return false; },
      [type](token::Operation &token) { return token.type() == type; }};
};

bool operator==(token::KwType type, token::token_union &token) {
  return std::visit(isKwVisitor(type), token);
}
bool operator!=(token::KwType type, token::token_union &token) {
  return !(type == token);
}

bool operator==(token::OpType type, token::token_union &token) {
  return std::visit(isOpVisitor(type), token);
}
bool operator!=(token::OpType type, token::token_union &token) {
  return !(type == token);
}

void Alphabet::init(token::tokens_list &tokens) {
  auto session = tokens.session();
  auto first = std::begin(tokens);

  if (token::KwType::ALPHABET != *first)
    std::visit(
        [](const auto &token) {
          throwMismatch(kwBimap.toString(token::KwType::ALPHABET), token);
        },
        *first);
  session.popFront();
  first = std::begin(tokens);
  if (!std::visit(isOpVisitor(token::OpType::SEMICOLON), *first))
    std::visit(
        [](const auto &token) {
          throwMismatch(opBimap.toString(token::OpType::SEMICOLON), token);
        },
        *first);
  session.popFront();

  first = std::begin(tokens);
  bool wasComa = false;
  while (!tokens.empty() &&
         !std::visit(isOpVisitor(token::OpType::TERMINATOR), *first)) {
  }

  session.commit();
}
