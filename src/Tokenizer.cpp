#include "Tokenizer.hpp"
#include "CharStream.hpp"
#include "Token.hpp"
#include "exception.hpp"
#include <cctype>
#include <utility>

/**
 * @brief Read token from stream. Assuming that no spaces before i.e. token begin at current position
 *
 * @param stream -- ICharStream realization
 *
 * @return one of tokens in union type
 */
token::token_union read_token(ICharStream & stream) {
	static auto [op, kw] = impl::initTries();
	auto opPoint = op.begin();
	auto kwPoint = kw.begin();
	size_t begin = stream.position();
	
	char c;
	stream >> c;
	if(opPoint->canGoTo(c)) {
		while(opPoint->canGoTo(c) && !opPoint->isTerm()) {
			opPoint->goTo(c);
			if(stream.eof()) 
				break;
			stream >> c;
		}
		stream << c;

		if(!opPoint->isTerm()) 
			throw error::UndefinedOperatorError(stream.position());

		auto opType = opPoint->get();
		return {token::Operation(opType, begin, stream.position())};
	}
	auto kwPointBegin = kw.begin();
	std::string buffer = "";

	while(!stream.eof() && !isspace(c) && !opPoint->canGoTo(c)) {
		if(*kwPoint != *kwPointBegin && kwPoint->canGoTo(c)) 
				kwPoint->goTo(c);
		buffer.push_back(c);
		stream >> c;
	}

	if(!stream.eof()) 
		stream << c;

	if(kwPoint->isTerm()) 
		return {token::Keyword(kwPoint->get(), begin, stream.position())};
	return {token::Name(buffer, begin, stream.position())};
}

token::Tokenizer::return_type
token::Tokenizer::parse(ICharStream & stream) {
	char c;
	token::Tokenizer::return_type result;
	while(!stream.eof()) {
		stream >> c;
		if(isspace(c)) 
			continue;
		stream << c;
		result.push_back(read_token(stream));
	}

	return result;
}
