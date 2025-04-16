#include "AST.hpp"
#include "Token.hpp"
#include "Tokenizer.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <iterator>
#include <optional>
#include <variant>

using namespace ast;

ast::Name::Name(token::tokens_list &tokens, std::string name) : NodeBase(ExprType::NAME), name_(name) {
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

Alphabet::Alphabet(token::tokens_list & tokens)
	: NodeBase(ExprType::ALPHABET){
	init(tokens);
}

// -----------------------
// | init overloads next |
// -----------------------

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

template <class T>
void throwMismatch(const std::string &expected, const T& given) {
	throw error::ExpectedMismatchError(given.begin(), expected, given.toString());
}

void Name::init(token::tokens_list & tokens) {
	std::string value;
	auto visitor = overloads {
		[&value](token::Name & token) {
			value = token.toString();
		},
		[](token::Keyword & token) { throwMismatch(token::Name::typeToString(), token); },
		[](token::Operation & token) { throwMismatch(token::Name::typeToString(), token); }
	};

	std::visit(visitor, *std::begin(tokens));
	tokens.pop_front();
	name_ = value;
}

void Alphabet::init(token::tokens_list & tokens) {
	auto first = std::begin(tokens);


}
