#ifndef TOKENIZER_HPP_
#define TOKENIZER_HPP_

#include "CharStream.hpp"
#include "Token.hpp"
#include <list>
#include <queue>
#include <variant>
namespace token {
class token_union;
class tokens_list : public std::list<token_union> {
	bool transactionBegan_ = false;
	class ctx {
		std::queue<token_union> cache_;
		tokens_list * root_;
	public:
		ctx(tokens_list *root) : root_(root) {}
		void commit() {
			while (!cache_.empty()) 
				cache_.pop();
		};

		void popFront() {
			cache_.push(*std::begin(*root_));
			root_->pop_front();
		}

		~ctx() {
			while (!cache_.empty()) {
				root_->push_front(cache_.front());
				cache_.pop();
			}
		}
	};
public:
	ctx session() {return ctx{this};}
};

class ITokenizer {
public:
	virtual tokens_list parse(ICharStream & stream) = 0;
};

class Tokenizer : public ITokenizer {
	public:
		tokens_list parse(ICharStream & stream) override;
};

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

bool operator==(const KwType, const token_union &);
bool operator==(const OpType, const token_union &);
bool operator!=(const KwType, const token_union &);
bool operator!=(const OpType, const token_union &);

} // namespace token
#endif // !TOKENIZER_HPP_
