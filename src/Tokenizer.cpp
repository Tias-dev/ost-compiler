#include "Tokenizer.hpp"
#include "CharStream.hpp"
#include "Token.hpp"
#include "trie.hpp"
#include <utility>


using OpTrie = impl::Trie<token::OpType>;
using KwTrie = impl::Trie<token::KwType>;

std::pair<OpTrie, KwTrie> initTries() {
	KwTrie kw;
	kw.add("MT", token::KwType::MT);
	kw.add("BEGIN", token::KwType::BEGIN);
	kw.add("END", token::KwType::END);
	kw.add("ALPHABET", token::KwType::ALPHABET);
	kw.add("IF", token::KwType::IF);
	kw.add("FI", token::KwType::FI);
	kw.add("DO", token::KwType::DO);
	kw.add("OD", token::KwType::OD);

	OpTrie op;
	op.add(";", token::OpType::TERMINATOR);
	op.add(",", token::OpType::COMA);
	op.add(":", token::OpType::SEMICOLON);
	op.add("a", token::OpType::SET_LETTER);
	op.add("(", token::OpType::LEFT_BRACKET);
	op.add(")", token::OpType::RIGHT_BRACKET);
	op.add("_", token::OpType::LAMBDA);
	op.add("**", token::OpType::POW);
	op.add("?", token::OpType::QUESTION);
	op.add("!=", token::OpType::NOT_EQUAL);

	return {op, kw};
}

token::Tokenizer::return_type
parse(ICharStream & stream) {
	auto [kw, op] = initTries();
}
