#include "AST.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <compare>
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

Paired::Paired(token::tokens_list &tokens, token::KwType first,
               token::KwType second, bool forceTerminator)
    : NodeBase(ExprType::PAIRED), first_(first), second_(second),
      forceTerminator_(forceTerminator) {
  init(tokens);
};

Alphabet::Alphabet(token::tokens_list &tokens) : NodeBase(ExprType::ALPHABET) {
  init(tokens);
}

MT::Call::Call(token::tokens_list &tokens)
    : NodeBase(ExprType::MT), id_(-1) {
  init(tokens);
	if(id_ == -1) 
		throw std::logic_error("After init MT::Call don't change it's id to actual");
}

MT::Lib::Lib(token::tokens_list &tokens)
    : NodeBase(ExprType::MT), id_(currentId_++) {
  init(tokens);
}


MT::Definition::Definition(token::tokens_list &tokens)
    : NodeBase(ExprType::MT), id_(currentId_++) {
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

void MT::Call::init(token::tokens_list & tokens) {
	auto session = tokens.session();
	auto token = session.popFrontAndReturn();

	if(!token.isName() || !namesTable_.contains(token.getName())) 
    throwSemantic("Expected already defined mt name", token.begin());
	
	id_ = namesTable_[token.getName()];

	token = session.popFrontAndReturn();
	if(token != token::OpType::TERMINATOR) 
		throwMismatch(";", token);

	session.commit();
}

void MT::Lib::init(token::tokens_list & tokens) {
	auto session = tokens.session();
	auto token = session.popFrontAndReturn();

	if(token != token::KwType::MT) 
		throwMismatch("MT", token);

	token = session.popFrontAndReturn();
	if(!token.isName()) 
    throwSemantic("valid mt name", token.begin());
	
	std::string name = token.getName();
	if(namesTable_.contains(name)) 
		throw error::RedefinitionError(token);
		
	namesTable_[name] = currentId_;

	token = session.popFrontAndReturn();
	if(token != token::OpType::TERMINATOR)
		throwMismatch(";", token);
	
	token = session.popFrontAndReturn();
	if(token != token::KwType::LIB)
		throwMismatch("LIB", token);

	token = session.popFrontAndReturn();
	if(token != token::OpType::TERMINATOR)
		throwMismatch(";", token);
		
	session.commit();
}

