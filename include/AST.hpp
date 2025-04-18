#ifndef AST_HPP_
#define AST_HPP_

#include "Token.hpp"
#include "Tokenizer.hpp"
#include "utils.hpp"
#include <list>
#include <optional>
#include <set>
#include <string>

namespace ast {
enum class ExprType { NAME, ALPHABET, PAIRED, MT };

class NodeBase {
protected:
  std::list<NodeBase *> childs_;
  ExprType type_;
  virtual void init(token::tokens_list &tokens) = 0;

public:
  NodeBase(ExprType type) : type_(type) {}
};


class Name : public NodeBase {
  std::string name_;
  size_t id_;

  static size_t counterId;
  static bimap<std::string, size_t> namesTable_;
  void init(token::tokens_list &tokens) override;
public:
  Name(token::tokens_list &tokens, std::string name);
  const std::string &name() { return name_; }
  size_t id() { return id_; }

  static std::optional<size_t> getNameId(const std::string &name);
  static std::optional<std::string> getNameById(const size_t id);
};

class Alphabet : public NodeBase {
	std::set<char> alphabet_;

	void init(token::tokens_list & tokens) override;
public:
		Alphabet(token::tokens_list & tokens);
};

class Paired : public NodeBase {
  token::KwType first_;
  token::KwType second_;
  bool forceTerminator_ = true;
  void init(token::tokens_list &tokens) override;

protected:
  Paired(token::tokens_list &tokens, token::KwType first, token::KwType second,
         bool forceTerminator = true);
};

class BeginEnd : public Paired {
public:
  BeginEnd(token::tokens_list &tokens)
      : Paired(tokens, token::KwType::BEGIN, token::KwType::END, false) {}
};

class If : public Paired {
public:
  If(token::tokens_list &tokens)
      : Paired(tokens, token::KwType::IF, token::KwType::FI, false) {}
};

class Do : public Paired {
public:
  Do(token::tokens_list &tokens)
      : Paired(tokens, token::KwType::DO, token::KwType::OD, true) {}
};

class MT : public NodeBase {
  bool isLib = false;
  void init(token::tokens_list &tokens) override;

  static bimap<std::string, size_t> namesTable_;

public:
  MT(token::tokens_list &tokens);

  static bool isMTName(const std::string &name) {
    return namesTable_.contains(name);
  }
};

class Tree {
  NodeBase *root = nullptr;

public:
  Tree(token::tokens_list &tokens);
};
} // namespace ast

#endif // !AST_HPP_
