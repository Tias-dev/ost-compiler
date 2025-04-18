#ifndef TOKENIZER_HPP_
#define TOKENIZER_HPP_

#include "CharStream.hpp"
#include "Token.hpp"
#include <list>
#include <stack>
#include <variant>

namespace token {
class token_union {
	using data_type = std::variant<token::Name, token::Keyword, token::Operation>;
	data_type data_;
public:
	token_union(data_type data) : data_(data) {}

	bool operator==(const KwType) const;
	bool operator==(const OpType) const;
	bool operator!=(const KwType) const;
	bool operator!=(const OpType) const;

	bool isName() const;
	std::string getName() const;
	
	size_t begin() const;
	size_t end() const;

	std::string toString();
	std::string typeToString();
};

class tokens_list : private std::list<token_union> {
	using parent = std::list<token_union>;

	class Session {
		std::stack<token_union> cache_;
		tokens_list * root_;
	public:
		Session(tokens_list *root) : root_(root) {}
		~Session();

		void commit();
		void rollback();
		void popFront();
		token_union popFrontAndReturn();
	};
	using parent::pop_front;
	using parent::push_front;
	using parent::pop_back;
	friend Session;
public:
	using parent::empty;
	using parent::begin; 
	using parent::end; 
	using parent::push_back;

	Session session() {return Session{this};}
};

class ITokenizer {
public:
	virtual tokens_list parse(ICharStream & stream) = 0;
};

class Tokenizer : public ITokenizer {
	public:
		tokens_list parse(ICharStream & stream) override;
};


bool operator==(const KwType, const token_union &);
bool operator==(const OpType, const token_union &);
bool operator!=(const KwType, const token_union &);
bool operator!=(const OpType, const token_union &);

} // namespace token
#endif // !TOKENIZER_HPP_
