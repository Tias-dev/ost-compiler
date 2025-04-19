#ifndef AST_HPP_
#define AST_HPP_

#include "Tokenizer.hpp"
#include "utils.hpp"
#include <list>
#include <optional>
#include <set>
#include <string>

namespace ast {
enum class ExprType { NAME, ALPHABET, PAIRED, MT, SET_LETTER };

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

  void init(token::tokens_list &tokens) override;

public:
  Alphabet(token::tokens_list &tokens);
};

class BeginEnd : public NodeBase{
	void init(token::tokens_list &tokens) override;
public:
  BeginEnd(token::tokens_list &tokens);
};

class IfFi : public NodeBase {
	void init(token::tokens_list &tokens) override;
	class Branch : public NodeBase {
		char letterToCheck_ = 0;
		void init(token::tokens_list &tokens) override;
	public:
		Branch(token::tokens_list & tokens);
	};
public:
  IfFi(token::tokens_list &tokens);
};

class DoOd : public NodeBase {
	void init(token::tokens_list &tokens) override;
	class Branch : public NodeBase {
		char letterToCheck_ = 0;
		bool isAnyChar_ = false;
		void init(token::tokens_list &tokens) override;
	public:
		Branch(token::tokens_list & tokens);
	};
public:
  DoOd(token::tokens_list &tokens);
};

class MT : public NodeBase {
  enum class Usage { DEFINITION, LIB, CALL };

  class Call : public NodeBase {
    size_t id_;
		size_t pow_ = 1;
		void init(token::tokens_list &tokens) override;
  public:
    Call(token::tokens_list &tokens);
    size_t id() { return id_; }
  };

	class Lib : public NodeBase {
    size_t id_;
		void init(token::tokens_list &tokens) override;
  public:
    Lib(token::tokens_list &tokens);
    size_t id() { return id_; }
	};

	class Definition : public NodeBase {
    size_t id_;
		void init(token::tokens_list &tokens) override;
  public:
    Definition(token::tokens_list &tokens);
    size_t id() { return id_; }
	};

  Usage usage_;
  void init(token::tokens_list &tokens) override;
  static bimap<std::string, size_t> namesTable_;
	static size_t currentId_;

public:
  MT(token::tokens_list &tokens);

  static bool isMTName(const std::string &name) {
    return namesTable_.contains(name);
  }
};

class SetLetter : public NodeBase {
	void init(token::tokens_list &tokens) override;
	char letter_ = 0;
public:
	SetLetter(token::tokens_list & tokens);
};

class Tree {
  NodeBase *root = nullptr;
public:
  Tree(token::tokens_list &tokens);
};
} // namespace ast

#endif // !AST_HPP_
