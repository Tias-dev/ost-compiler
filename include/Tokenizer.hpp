#ifndef TOKENIZER_HPP_
#define TOKENIZER_HPP_

#include "CharStream.hpp"
#include "Token.hpp"
#include <list>
#include <queue>
#include <variant>
namespace token {
using token_union = std::variant<token::Name, token::Keyword, token::Operation>;
class tokens_list : public std::list<token_union> {
	bool transactionBegan_ = false;
	class ctx {
		std::queue<token_union> cache_;
		tokens_list * root_;

	public:
		void commit() {
			while (!cache_.empty()) 
				cache_.pop();
		};
		~ctx() {
			while (!cache_.empty()) {
				root_->push_front(cache_.front());
				cache_.pop();
			}
		}
	};
public:
	void popFront();
};

class ITokenizer {
public:
	virtual tokens_list parse(ICharStream & stream) = 0;
};

class Tokenizer : public ITokenizer {
	public:
		tokens_list parse(ICharStream & stream) override;
};
} // namespace token

#endif // !TOKENIZER_HPP_
