#ifndef TOKENIZER_HPP_
#define TOKENIZER_HPP_

#include "CharStream.hpp"
#include "Token.hpp"
#include <list>
#include <variant>
namespace token {
using token_union = std::variant<token::Name, token::Keyword, token::Operation>;
using tokens_list = std::list<token_union>;

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
