#include "AST.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <chrono>
#include <iterator>
#include <optional>
#include <stdexcept>

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

BeginEnd::BeginEnd(token::tokens_list &tokens) : NodeBase(ExprType::PAIRED) {
  init(tokens);
}

DoOd::DoOd(token::tokens_list &tokens) : NodeBase(ExprType::PAIRED) {
  init(tokens);
}

IfFi::IfFi(token::tokens_list &tokens) : NodeBase(ExprType::PAIRED) {
  init(tokens);
}

Alphabet::Alphabet(token::tokens_list &tokens) : NodeBase(ExprType::ALPHABET) {
  init(tokens);
}

MT::Call::Call(token::tokens_list &tokens) : NodeBase(ExprType::MT), id_(-1) {
  init(tokens);
  if (id_ == -1)
    throw std::logic_error(
        "After init MT::Call don't change it's id to actual");
}

MT::Lib::Lib(token::tokens_list &tokens)
    : NodeBase(ExprType::MT), id_(currentId_++) {
  init(tokens);
}

MT::Definition::Definition(token::tokens_list &tokens)
    : NodeBase(ExprType::MT), id_(currentId_++) {
  init(tokens);
}

SetLetter::SetLetter(token::tokens_list &tokens)
    : NodeBase(ExprType::SET_LETTER) {
  init(tokens);
}

// --------------------------
// | exception wrapper next |
// --------------------------

void throwMismatch(const std::string &expected, token::token_union &given) {
  throw error::ExpectedMismatchError(given.begin(), expected, given.toString());
}

void throwSemantic(const std::string &what, size_t position) {
  throw error::SemanticError(position, what);
}

// -------------------------
// |  ast nodes init next  |
// -------------------------

void Name::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  name_ = std::begin(tokens)->getName();
  session.popFront();

  session.commit();
}

void Alphabet::init(token::tokens_list &tokens) {
  auto session = tokens.session();
  auto token = *std::begin(tokens);

  if (token::KwType::ALPHABET != token)
    throwMismatch("ALPHABET", token);
  session.popFront();

  token = *std::begin(tokens);
  if (token::OpType::SEMICOLON != token)
    throwMismatch(":", token);
  session.popFront();

  bool wasComa = false;
  while (!tokens.empty() && token::OpType::TERMINATOR != token) {
    token = *std::begin(tokens);
    if (token::OpType::COMA == token) {
      if (wasComa)
        throwSemantic("Multiple comas in alphabet definition", token.begin());
      else
        wasComa = true;
    } else {
      std::string name = token.getName();
      if (name.size() > 1)
        throwSemantic("Alphabet items must be letters not words",
                      token.begin());
      alphabet_.insert(name[0]);
      wasComa = false;
    }
    session.popFront();
  }

  if (tokens.empty() || (*std::begin(tokens) != token::OpType::TERMINATOR))
    throwSemantic("Alphabet definition must be finished by ;", token.end());

  session.popFront();
  session.commit();
}

void MT::init(token::tokens_list &tokens) {
  auto session = tokens.session();
  token::token_union token = session.popFrontAndReturn();

  if (token != token::KwType::MT) {
    if (token.isName() && MT::isMTName(token.getName())) {
      session.rollback();
      usage_ = Usage::CALL;
      childs_.push_back(new Call{tokens});
      return;
    }
    throwSemantic("Expected already defined mt name", token.begin());
  }

  token = session.popFrontAndReturn();
  if (!token.isName())
    throwMismatch("valid mt name", token);

  std::string name = token.getName();
  if (namesTable_.contains(name))
    throw error::RedefinitionError(token);

  token = session.popFrontAndReturn();
  if (token != token::OpType::TERMINATOR)
    throwMismatch(";", token);

  token = session.popFrontAndReturn();
  if (token == token::KwType::LIB || token == token::KwType::BEGIN) {
    session.rollback();

    if (token == token::KwType::LIB) {
      usage_ = Usage::LIB;
      childs_.push_back(new Lib{tokens});
    } else {
      usage_ = Usage::DEFINITION;
      childs_.push_back(new Definition{tokens});
    }
    return;
  }
}

std::optional<size_t> toNumber(const std::string &s) {
  size_t buffer = 0;
  for (auto &c : s) {
    if (c < '0' || c > '9')
      return std::nullopt;
    buffer = buffer * 10 + c - '0';
  }

  return {buffer};
}

void MT::Call::init(token::tokens_list &tokens) {
  auto session = tokens.session();
  auto token = session.popFrontAndReturn();

  if (!token.isName() || !namesTable_.contains(token.getName()))
    throwSemantic("Expected already defined mt name", token.begin());

  id_ = namesTable_[token.getName()];

  token = *std::begin(tokens);
  if (token == token::OpType::POW) {
    session.popFront();
    session.popFrontAndReturn();
    if (!token.isName())
      throwMismatch("positive pow number", token);

    auto pow = toNumber(token.getName());
    if (!pow.has_value())
      throwMismatch("positive pow number", token);

    pow_ = *pow;
    token = *std::begin(tokens);
  }

  if (token == token::OpType::TERMINATOR)
    session.popFront();

  session.commit();
}

void MT::Lib::init(token::tokens_list &tokens) {
  auto session = tokens.session();
  auto token = session.popFrontAndReturn();

  if (token != token::KwType::MT)
    throwMismatch("MT", token);

  token = session.popFrontAndReturn();
  if (!token.isName())
    throwSemantic("valid mt name", token.begin());

  std::string name = token.getName();
  if (namesTable_.contains(name))
    throw error::RedefinitionError(token);

  namesTable_[name] = currentId_;

  token = session.popFrontAndReturn();
  if (token != token::OpType::TERMINATOR)
    throwMismatch(";", token);

  token = session.popFrontAndReturn();
  if (token != token::KwType::LIB)
    throwMismatch("LIB", token);

  token = session.popFrontAndReturn();
  if (token != token::OpType::TERMINATOR)
    throwMismatch(";", token);

  session.commit();
}

void MT::Definition::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
  if (token != token::KwType::MT)
    throwMismatch("MT", token);

  token = session.popFrontAndReturn();
  if (!token.isName())
    throwMismatch("valid mt name", token);
  else if (namesTable_.contains(token.getName()))
    throw error::RedefinitionError(token);

  std::string name = token.getName();
  namesTable_[name] = id_;

  token = session.popFrontAndReturn();
  if (token != token::OpType::TERMINATOR)
    throwMismatch(";", token);

  session.commit();

  token = session.popFrontAndReturn();
  if (token != token::KwType::BEGIN)
    throwMismatch("BEGIN", token);

  session.rollback();
  childs_.push_back(new BeginEnd{tokens});
  session.commit();
}

void SetLetter::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
  if (token != token::OpType::SET_LETTER)
    throwMismatch("a", token);

  token = session.popFrontAndReturn();
  if (token != token::OpType::LEFT_BRACKET)
    throwMismatch("(", token);

  token = session.popFrontAndReturn();
  if (!(token == token::OpType::LAMBDA || token.isName()))
    throwMismatch("_ or letter", token);

  if (token == token::OpType::LAMBDA)
    letter_ = '_';
  else {
    std::string name = token.getName();
    if (name.size() > 1)
      throwMismatch("one letter", token);
    letter_ = name[0];
  }

  token = session.popFrontAndReturn();
  if (token != token::OpType::RIGHT_BRACKET)
    throwMismatch(")", token);

  session.commit();

  token = *std::begin(tokens);
  if (token == token::OpType::TERMINATOR)
    session.popFront();

  session.commit();
}

void BeginEnd::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
  if (token != token::KwType::BEGIN)
    throwMismatch("BEGIN", token);
  auto beginToken = token;
  session.commit();
  while (!tokens.empty() && token != token::KwType::END) {
    token = *std::begin(tokens);
    // --------------------------
    // |  keywords check first  |
    // --------------------------
    if (token == token::KwType::END)
      break;
    else if (token == token::KwType::BEGIN)
      childs_.push_back(new BeginEnd{tokens});
    else if (token == token::KwType::MT)
      childs_.push_back(new MT{tokens});
    else if (token == token::KwType::DO)
      childs_.push_back(new DoOd{tokens});
    else if (token == token::KwType::IF)
      childs_.push_back(new IfFi{tokens});
    else if (token == token::KwType::FI || token == token::KwType::OD)
      throw error::ClosingTokenBeforeOpenTokenError(token);
    // --------------------
    // |  mt calls check  |
    // --------------------
    else if (token.isName())
      childs_.push_back(new MT{tokens});
    // -----------------------------
    // |  operations check laslty  |
    // -----------------------------
    else if (token == token::OpType::TERMINATOR)
      session.popFront();
    else if (token == token::OpType::SET_LETTER)
      childs_.push_back(new SetLetter{tokens});
    // ----------------------------------------------
    // |  all unchecked cases -- unexpected tokens  |
    // ----------------------------------------------
    else
      throw error::UnexpectedTokenError(token);
  }

  if (tokens.empty())
    throw error::ClosingTokenNotFound(beginToken);

  session.commit();
}

void DoOd::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
  if (token != token::KwType::DO)
    throwMismatch("DO", token);

  auto beginToken = token;
  token = *std::begin(tokens);
  while (!tokens.empty() && token != token::KwType::OD) {
    childs_.push_back(new Branch{tokens});
    token = *std::begin(tokens);
  }

  if (tokens.empty())
    throw error::ClosingTokenNotFound(beginToken);

  session.popFront();
  session.commit();
}

DoOd::Branch::Branch(token::tokens_list &tokens) : NodeBase(ExprType::PAIRED) {
  init(tokens);
}

void DoOd::Branch::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
  if (token == token::OpType::LEFT_BRACKET) {
    token = session.popFrontAndReturn();
    if (token != token::OpType::RIGHT_BRACKET)
      throwMismatch(")", token);

    token = session.popFrontAndReturn();
    if (token != token::OpType::NOT_EQUAL)
      throwMismatch("!=", token);
    isAnyChar_ = true;
    token = session.popFrontAndReturn();
  }

  if (!token.isName() || token.getName().size() > 1)
    throwMismatch("letter to check", token);

  letterToCheck_ = token.getName()[0];

  token = session.popFrontAndReturn();
  if (token != token::OpType::QUESTION)
    throwMismatch("?", token);

  auto first = std::begin(tokens), second = std::next(first);
  while (tokens.size() > 2 &&
         !(*first == token::KwType::OD ||
					 *second == token::OpType::QUESTION ||
           *second == token::OpType::RIGHT_BRACKET)
				 ) {
    if (*first == token::OpType::SET_LETTER)
      childs_.push_back(new SetLetter{tokens});
    else
      childs_.push_back(new MT{tokens});

    first = std::begin(tokens);
    second = std::next(first);
  }
  session.commit();
}
