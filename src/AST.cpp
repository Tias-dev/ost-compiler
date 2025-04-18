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

// -------------------------------
// | token union operations next |
// -------------------------------


void throwMismatch(const std::string &expected, token::token_union &given) {
			throw error::ExpectedMismatchError(given.begin(), expected,
																				 given.toString());
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
        throwSemantic( "Multiple comas in alphabet definition", token.begin());
			else 
				wasComa = true;
    } else {
			std::string name = token.getName();
			if(name.size() > 1) throwSemantic("Alphabet items must be letters not words", token.begin());
			alphabet_.insert(name[0]);	
			wasComa = false;
		}
		session.popFront();
  }

	if(tokens.empty() || (*std::begin(tokens) != token::OpType::TERMINATOR)) 
		throwSemantic("Alphabet definition must be finished by ;", token.end());	
	
	session.popFront();
	session.commit();
}
