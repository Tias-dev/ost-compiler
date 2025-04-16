#include "AST.hpp"
#include "Tokenizer.hpp"
#include <optional>

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

// -----------------------
// | init overloads next |
// -----------------------
