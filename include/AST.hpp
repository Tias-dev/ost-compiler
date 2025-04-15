#ifndef AST_HPP_
#define AST_HPP_

#include "Token.hpp"
#include "utils.hpp"
#include <list>
#include <string>

namespace ast {
enum class ExprType { NAME, CONSTANT, ALPHABET, PAIRED, MT };

class NodeBase {
protected:
  std::list<NodeBase *> childs_;
  ExprType type_;

public:
  NodeBase(ExprType type) : type_(type) {}
};

class Name : public NodeBase {
  std::string name_;
  size_t id_;

  static size_t counterId;
  static bimap<std::string, size_t> namesTable_;

public:
  Name(std::string name) : NodeBase(ExprType::NAME), name_(name) {
    if (namesTable_.contains(name))
      id_ = namesTable_[name];
    else {
      id_ = counterId++;
      namesTable_.add(name_, id_);
    }
  }

  const std::string &name() { return name_; }
  size_t id() { return id_; }
};

class Constant : public NodeBase {
  char value_;

  static size_t counterId;
  static bimap<std::string, size_t> namesTable_;

public:
  Constant(char value) : NodeBase(ExprType::CONSTANT), value_(value) {}

  char value() { return value_; }
};


class Paired : public NodeBase {
	token::KwType first_;
	token::KwType second_;
	bool forceTerminator_ = true;
public:
	Paired();
};

class Tree {
  NodeBase *root;
};
} // namespace ast

#endif // !AST_HPP_
