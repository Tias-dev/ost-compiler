#ifndef AST_HPP_
#define AST_HPP_

#include "Tokenizer.hpp"
#include "Tu4Command.hpp"
#include "utils.hpp"
#include <iostream>
#include <list>
#include <optional>
#include <ostream>
#include <set>
#include <string>

namespace ast {
enum class ExprType { ALPHABET, PAIRED, MT, SET_LETTER };

class NodeBase {
protected:
  std::list<NodeBase *> childs_;
  ExprType type_;
  virtual void init(token::tokens_list &tokens) = 0;

public:
  NodeBase(ExprType type) : type_(type) {}

  virtual std::string toString() = 0;
  void print(std::ostream &os, size_t depth = 0);
  virtual std::list<tu4::tu4_union<size_t>> to4(const std::set<char> & alphabet, size_t & q0) = 0;
};

class Alphabet : public NodeBase {
  std::set<char> alphabet_ = {'_', 'T', 'F'};

  void init(token::tokens_list &tokens) override;

public:
  Alphabet(token::tokens_list &tokens);

  std::string toString() override;
	const std::set<char>& alphabet() {return alphabet_;}
	std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t &q0) override {return {};}
};

class BeginEnd : public NodeBase {
  void init(token::tokens_list &tokens) override;
  std::optional<token::token_union> endMT_;

public:
  BeginEnd(token::tokens_list &tokens);

  const token::token_union &endMTData() { return *endMT_; }
  std::string toString() override;
	std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t &q0) override;
};

class IfFi : public NodeBase {
  void init(token::tokens_list &tokens) override;

  class Branch : public NodeBase {
    char letterToCheck_ = 0;
    void init(token::tokens_list &tokens) override;

  public:
    Branch(token::tokens_list &tokens);

    std::string toString() override;
		std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t &q0) override;
  };

public:
  IfFi(token::tokens_list &tokens);

  std::string toString() override;
	std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t &q0) override;
};

class DoOd : public NodeBase {
  void init(token::tokens_list &tokens) override;

  class Branch : public NodeBase {
    char letterToCheck_ = 0;
    bool isAnyChar_ = false;
    void init(token::tokens_list &tokens) override;
		size_t to4_impl(std::list<tu4::tu4_union<size_t>> & commands, char letter, const std::set<char> &alphabet, const size_t &q0);

  public:
    Branch(token::tokens_list &tokens);

    std::string toString() override;
		std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t &q0) override;
  };

public:
  DoOd(token::tokens_list &tokens);

  std::string toString() override;
	std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t &q0) override;
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
		std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t & q0) override;
  };

  class Lib : public NodeBase {
    size_t id_;
    void init(token::tokens_list &tokens) override;

  public:
    Lib(token::tokens_list &tokens);

    std::string toString() override;
    size_t id() { return id_; }
		std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t & q0) override;
  };

  class Definition : public NodeBase {
    size_t id_;
    void init(token::tokens_list &tokens) override;
    Alphabet *alphabet_;

  public:
    Definition(token::tokens_list &tokens);

    size_t id() { return id_; }
    std::string toString() override;
		std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t & q0) override;
  };

  Usage usage_;
  void init(token::tokens_list &tokens) override;
  static bimap<std::string, size_t> namesTable_;
	static std::map<size_t, Definition*> definitons_;
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
	std::list<tu4::tu4_union<size_t>> to4(const std::set<char> &alphabet, size_t & q0) override;
};

class SetLetter : public NodeBase {
  void init(token::tokens_list &tokens) override;
  char letter_ = 0;

public:
  SetLetter(token::tokens_list &tokens);

  std::string toString() override;
};

class Tree {
  NodeBase *root_ = nullptr;

public:
  void print(std::ostream &os = std::cout);
  Tree(token::tokens_list &tokens);
};
} // namespace ast

#endif // !AST_HPP_
