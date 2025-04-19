#ifndef AST_HPP_
#define AST_HPP_

#include "Compiler.hpp"
#include "Tokenizer.hpp"
#include "utils.hpp"
#include <iostream>
#include <list>
#include <optional>
#include <ostream>
#include <set>
#include <string>

namespace ast {
enum class ExprType { ALPHABET, PAIRED, MT, SET_LETTER };
using compiler::commands_type;

class NodeBase {
protected:
  std::list<NodeBase *> childs_;
  ExprType type_;
  virtual void init(token::tokens_list &tokens) = 0;

public:
  NodeBase(ExprType type) : type_(type) {}

  virtual std::string toString() = 0;
  void print(std::ostream &os, size_t depth = 0);
  virtual commands_type to4(const compiler::Alphabet<char> &alphabet) = 0;
};

class Alphabet : public NodeBase {
  compiler::Alphabet<char> alphabet_{{'_', 'T', 'F'}};

  void init(token::tokens_list &tokens) override;

public:
  Alphabet(token::tokens_list &tokens);

  std::string toString() override;
  const compiler::Alphabet<char> &alphabet() { return alphabet_; }
  commands_type to4(const compiler::Alphabet<char> &alphabet) override {
    return {};
  }
};

class BeginEnd : public NodeBase {
  void init(token::tokens_list &tokens) override;
  std::optional<token::token_union> endMT_;

public:
  BeginEnd(token::tokens_list &tokens);

  const token::token_union &endMTData() { return *endMT_; }
  std::string toString() override;
  commands_type to4(const compiler::Alphabet<char> &alphabet) override;
};

class IfFi : public NodeBase {
  void init(token::tokens_list &tokens) override;

  class Branch : public NodeBase {
    void init(token::tokens_list &tokens) override;

  public:
    Branch(token::tokens_list &tokens);

    std::string toString() override;
    commands_type to4(const compiler::Alphabet<char> &alphabet) override;
  };

  Branch *true_ = nullptr, *false_ = nullptr;

public:
  IfFi(token::tokens_list &tokens);

  std::string toString() override;
  commands_type to4(const compiler::Alphabet<char> &alphabet) override;
};

class DoOd : public NodeBase {
  void init(token::tokens_list &tokens) override;

  class Branch : public NodeBase {
    char letterToCheck_ = 0;
    bool isAnyChar_ = false;
    void init(token::tokens_list &tokens) override;

  public:
    Branch(token::tokens_list &tokens);

    std::string toString() override;
    commands_type to4(const compiler::Alphabet<char> &alphabet) override;
    bool isAnyChar() const { return isAnyChar_; }
    char letterToCheck() const { return letterToCheck_; }
  };

  std::list<Branch *> branches_;

public:
  DoOd(token::tokens_list &tokens);

  std::string toString() override;
  commands_type to4(const compiler::Alphabet<char> &alphabet) override;
};

class MT : public NodeBase {
  enum class Usage { DEFINITION, LIB, CALL };

  class Call : public NodeBase {
    size_t id_;
    size_t pow_ = 1;
    void init(token::tokens_list &tokens) override;

  public:
    Call(token::tokens_list &tokens);

    std::string toString() override;
    size_t id() { return id_; }
    commands_type to4(const compiler::Alphabet<char> &alphabet) override;
  };

  class Lib : public NodeBase {
    size_t id_;
    void init(token::tokens_list &tokens) override;

  public:
    Lib(token::tokens_list &tokens);

    std::string toString() override;
    size_t id() { return id_; }
    commands_type to4(const compiler::Alphabet<char> &alphabet) override;
  };

  class Definition : public NodeBase {
    size_t id_;
    void init(token::tokens_list &tokens) override;
    Alphabet *alphabet_;

  public:
    Definition(token::tokens_list &tokens);

    size_t id() { return id_; }
    std::string toString() override;
    commands_type to4(const compiler::Alphabet<char> &alphabet) override;
  };

  Usage usage_;
	NodeBase * node_;
  void init(token::tokens_list &tokens) override;
  static bimap<std::string, size_t> namesTable_;
  static std::map<size_t, NodeBase *> definitions_;
  static size_t currentId_;
public:
  MT(token::tokens_list &tokens);

  static void printNamesTable(std::ostream &os = std::cout) {
    namesTable_.print();
  }

  static bool isMTName(const std::string &name) {
    return namesTable_.contains(name);
  }
  std::string toString() override;
  virtual commands_type to4(const compiler::Alphabet<char> &alphabet) override;
	commands_type forceTo4(const compiler::Alphabet<char> &alphabet) {
		return node_->to4(alphabet);
	}
};

class SetLetter : public NodeBase {
  void init(token::tokens_list &tokens) override;
  char letter_ = 0;

public:
  SetLetter(token::tokens_list &tokens);

  std::string toString() override;
	commands_type to4(const compiler::Alphabet<char> &alphabet) override;
};

class Tree {
  MT *root_ = nullptr;

public:
  void print(std::ostream &os = std::cout);
  Tree(token::tokens_list &tokens, const std::string & fileName);
	commands_type to4();
};
} // namespace ast

#endif // !AST_HPP_
