#include "AST.hpp"
#include "Compiler.hpp"
#include "FilePosition.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <iostream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>

using namespace ast;
void throwMismatch(const std::string &expected,
                   const token::token_union &given);

Tree::Tree(token::tokens_list &tokens, const std::string &fileName) {
  try {
    root_ = new MT::Definition{tokens};
  } catch (error::PositionErrorBase &e) {
		logger::error() << "Error: " << e.what() << std::endl;
    auto& position = e.position(); size_t width = 60;
		fileRollAround(fileName, position, width);
		throw &e;
  }
  if (!tokens.empty())
    throwMismatch("End of file", *std::begin(tokens));
}


bimap<std::string, size_t> MT::namesTable_{{{"l", 0}, {"r", 1}}};
size_t MT::currentId_ = MT::namesTable_.size();

MT::MT(token::tokens_list &tokens) : NodeBase(ExprType::MT) {
  init(tokens);
  node_ = *std::begin(childs_);
}

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
  definitions_[id_] = this;
}

MT::Definition::Definition(token::tokens_list &tokens)
    : NodeBase(ExprType::MT), id_(currentId_++) {
  init(tokens);
  definitions_[id_] = this;
}

SetLetter::SetLetter(token::tokens_list &tokens)
    : NodeBase(ExprType::SET_LETTER) {
  init(tokens);
}

Branch::Branch(token::tokens_list &tokens) : NodeBase(ExprType::PAIRED) {
  init(tokens);
}

// --------------------------
// | exception wrapper next |
// --------------------------

void throwMismatch(const std::string &expected,
                   const token::token_union &given) {
  throw error::ExpectedMismatchError(given.begin(), expected, given.toString());
}

void throwSemantic(const std::string &what, const FilePosition & position) {
  throw error::SemanticError(position, what);
}

// -------------------------
// |  ast nodes init next  |
// -------------------------

void Alphabet::init(token::tokens_list &tokens) {
  auto session = tokens.session();
  auto token = *std::begin(tokens);

  if (token::KwType::ALPHABET != token)
    throwMismatch("ALPHABET", token);
	begin_ = token.begin();
  session.popFront();

  token = *std::begin(tokens);
  if (token::OpType::SEMICOLON != token)
    throwMismatch(":", token);
  session.popFront();

  bool wasComa = false;
  token = *std::begin(tokens);
  while (!tokens.empty() && token::OpType::TERMINATOR != token) {
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
    token = *std::begin(tokens);
  }

  if (tokens.empty() || (*std::begin(tokens) != token::OpType::TERMINATOR))
    throwSemantic("Alphabet definition must be finished by ;", token.end());
	end_ = token.begin();

  session.popFront();
  session.commit();
}

void MT::init(token::tokens_list &tokens) {
  auto session = tokens.session();
  token::token_union token = session.popFrontAndReturn();
	begin_ = token.begin();
	usage_ = Usage::OTHER;

  if (token == token::KwType::DO) {
    session.rollback();
    node_ = new DoOd{tokens};
		childs_.push_back(node_);
		end_ = node_->end();
    return;
  }

  if (token == token::KwType::IF) {
    session.rollback();
    node_ = new IfFi{tokens};
		childs_.push_back(node_);
		end_ = node_->end();
    return;
  }

  if (token == token::KwType::SET_LETTER) {
    session.rollback();
    node_ = new SetLetter{tokens};
		childs_.push_back(node_);
		end_ = node_->end();
    return;
  }

  if (token != token::KwType::MT) {
    if (token.isName() && MT::isMTName(token.getName())) {
      session.rollback();
      usage_ = Usage::CALL;
      childs_.push_back(node_ = new Call{tokens});
			end_ = node_->end();
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
      childs_.push_back(node_ = new Lib{tokens});
    } else {
      usage_ = Usage::DEFINITION;
      childs_.push_back(node_ = new Definition{tokens});
    }
		end_ = node_->end();
    return;
  }
	end_ = node_->end();
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
	begin_ = token.begin();
	end_ = token.end();

  id_ = namesTable_[token.getName()];

  token = *std::begin(tokens);
  if (token == token::OpType::POW) {
    session.popFront();
    token = session.popFrontAndReturn();
    if (!token.isName())
      throwMismatch("positive pow number", token);

    auto pow = toNumber(token.getName());
    if (!pow.has_value())
      throwMismatch("positive pow number", token);

    pow_ = *pow;
		end_ = token.end();
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

	begin_ = token.begin();

  token = session.popFrontAndReturn();
  if (!token.isName())
    throwSemantic("valid mt name", token.begin());

  std::string name = token.getName();
  if (namesTable_.contains(name))
    throw error::RedefinitionError(token);

  namesTable_.add(name, id_);

  token = session.popFrontAndReturn();
  if (token != token::OpType::TERMINATOR)
    throwMismatch(";", token);

  token = session.popFrontAndReturn();
  if (token != token::KwType::LIB)
    throwMismatch("LIB", token);

	end_ = token.end();
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
	begin_ = token.begin();

  token = session.popFrontAndReturn();
  if (!token.isName())
    throwMismatch("valid mt name", token);
  else if (namesTable_.contains(token.getName()))
    throw error::RedefinitionError(token);

  std::string name = token.getName();
  namesTable_.add(name, id_);

  token = session.popFrontAndReturn();
  if (token != token::OpType::TERMINATOR)
    throwMismatch(";", token);

  session.commit();

  token = session.popFrontAndReturn();
  if (token != token::KwType::BEGIN)
    throwMismatch("BEGIN", token);

  token = *std::begin(tokens);
  if (token != token::KwType::ALPHABET)
    throwMismatch("ALPHABET", token);

  alphabet_ = new Alphabet{tokens};
  session.rollback();

  BeginEnd *body;
  childs_.push_back(body = new BeginEnd{tokens});
  auto &endToken = body->endMTData();
  if (!endToken.isName() || endToken.getName() != name)
    throwMismatch(name, endToken);
	
	end_ = endToken.end();
  session.commit();

}

void SetLetter::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
  if (token != token::KwType::SET_LETTER)
    throwMismatch("a", token);

	begin_ = token.begin();

  token = session.popFrontAndReturn();
  if (token != token::OpType::LEFT_BRACKET)
    throwMismatch("(", token);

  token = session.popFrontAndReturn();
  if (!(token == token::KwType::LAMBDA || token.isName()))
    throwMismatch("_ or letter", token);

  if (token == token::KwType::LAMBDA)
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

	end_ = token.end();

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
	begin_ = token.begin();
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
    else if (token == token::KwType::ALPHABET)
      childs_.push_back(new Alphabet{tokens});
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
    else if (token == token::KwType::SET_LETTER)
      childs_.push_back(new SetLetter{tokens});
    // ----------------------------------------------
    // |  all unchecked cases -- unexpected tokens  |
    // ----------------------------------------------
    else
      throw error::UnexpectedTokenError(token);
  }

  if (tokens.empty())
    throw error::ClosingTokenNotFound(beginToken);

  token = session.popFrontAndReturn();
  if (token != token::KwType::END)
    throwMismatch("END", token);

  token = session.popFrontAndReturn();
  if (!token.isName())
    throwMismatch("MT name", token);
  endMT_ = token;
	end_ = endMT_->end();

  if (!tokens.empty() && *std::begin(tokens) == token::OpType::TERMINATOR)
    session.popFront();

  session.commit();
}

void DoOd::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
  if (token != token::KwType::DO)
    throwMismatch("DO", token);

	begin_ = token.begin();
  auto beginToken = token;
  token = *std::begin(tokens);
  while (!tokens.empty() && token != token::KwType::OD) {
    branches_.push_back(new Branch{tokens});
    token = *std::begin(tokens);
  }

  if (tokens.empty())
    throw error::ClosingTokenNotFound(beginToken);

	end_ = token.end();
  session.popFront();
  token = session.popFrontAndReturn();
  if (token != token::OpType::TERMINATOR)
    throwMismatch(";", token);

  session.commit();
  for (auto &branch : branches_)
    childs_.push_back(branch);

}

void Branch::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
	begin_ = token.begin();
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

  if (token == token::KwType::LAMBDA)
    letterToCheck_ = '_';
  else {
    if (!token.isName() || token.getName().size() > 1)
      throwMismatch("letter to check", token);
    letterToCheck_ = token.getName()[0];
  }

  token = session.popFrontAndReturn();
  if (token != token::OpType::QUESTION)
    throwMismatch("?", token);

  auto first = std::begin(tokens), second = std::next(first);
  while (tokens.size() > 2 &&
         !(*first == token::KwType::FI || *first == token::KwType::OD || *second == token::OpType::QUESTION ||
           *second == token::OpType::RIGHT_BRACKET)) {
    childs_.push_back(new MT{tokens});
		end_ = (*std::prev(std::end(childs_)))->end();

    first = std::begin(tokens);
    second = std::next(first);
  }
  session.commit();
}

void IfFi::init(token::tokens_list &tokens) {
  auto session = tokens.session();

  auto token = session.popFrontAndReturn();
	begin_ = token.begin();
  if (token != token::KwType::IF)
    throwMismatch("IF", token);

  auto tokenBegin = *std::begin(tokens);
  token = *std::begin(tokens);
  while (tokens.size() > 0 && token != token::KwType::FI) {
		auto branch = new Branch{tokens};
    branches_.push_back(branch);
		childs_.push_back(branch);
    token = *std::begin(tokens);
  }
  if (tokens.size() == 0)
    throw error::UnexpectedFileEnd("FI");

	end_ = token.end();
  session.popFront();
  if (tokens.size() > 0 && *std::begin(tokens) == token::OpType::TERMINATOR)
    session.popFront();

  session.commit();

}

// -----------------------------
// |  printing functions next  |
// -----------------------------

void Tree::print(std::ostream &os) { root_->print(os, 0); }

void NodeBase::print(std::ostream &os, size_t depth) {
  for (size_t i = 0; i < depth; ++i)
    os << "  ";
  os << toString() << " begin: " << begin_.to_string() << " end: " << end_.to_string() << std::endl;
  for (auto &elem : childs_)
    elem->print(os, depth + 1);
}

std::string MT::toString() {
  strfast ss;
  ss << "MT: ";
  switch (usage_) {
	case Usage::OTHER:
		ss << "OTHER";
		break;
  case Usage::DEFINITION:
    ss << "Definition";
    break;
  case Usage::LIB:
    ss << "Lib";
    break;
  case Usage::CALL:
    ss << "Call";
  }

  return ss.bump();
}

std::string MT::Call::toString() {
  strfast ss;
  ss << "MT: " << namesTable_[id_];
  if (pow_ > 1)
    ss << "**" << pow_;
  return ss.bump();
}

std::string MT::Lib::toString() {
  strfast ss;
  ss << "MT: " << namesTable_[id_];
  return ss.bump();
}

std::string MT::Definition::toString() {
  strfast ss;
  ss << "MT: " << namesTable_[id_] << "; " << alphabet_->toString();
  return ss.bump();
}

std::string Alphabet::toString() {
  strfast ss;
  ss << "ALPHABET: ";
  for (auto &c : alphabet_)
    ss << c << ", ";

  return ss.bump();
}

std::string DoOd::toString() {
  strfast ss;
  ss << "DO ... OD";
  return ss.bump();
}

std::string Branch::toString() {
  strfast ss;
  ss << "branch: ";
  if (isAnyChar_)
    ss << "Any char != " << letterToCheck_;
  else
    ss << "current char == " << letterToCheck_;

  return ss.bump();
}

std::string IfFi::toString() {
  strfast ss;
  ss << "IF ... FI";

  return ss.bump();
}

std::string BeginEnd::toString() {
  strfast ss;
  ss << "BEGIN ... END";

  return ss.bump();
}

std::string SetLetter::toString() {
  strfast ss;
  ss << "set [" << letter_ << "] letter";

  return ss.bump();
}
