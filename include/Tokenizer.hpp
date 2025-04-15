#ifndef TOKENIZER_HPP_
#define TOKENIZER_HPP_

#include "CharStream.hpp"
#include "Token.hpp"
#include <list>
#include <variant>
namespace token {
using token_union = std::variant<token::Name, token::Keyword, token::Operation>;

class ITokenizer {
public:
	using return_type = std::list<token_union>;
	virtual return_type parse(ICharStream & stream) = 0;
};

class Tokenizer : public ITokenizer {
	public:
		return_type parse(ICharStream & stream) override;
};
} // namespace token

#endif // !TOKENIZER_HPP_
