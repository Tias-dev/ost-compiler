#include "Tokenizer.hpp"
#include "CharStream.hpp"
#include "Token.hpp"
#include "exception.hpp"
#include <cctype>
#include <utility>

bool isSpace(int c) {
	return isspace(c) || (c == '"');
}

/**
 * @brief Read token from stream. Assuming that no spaces before i.e. token
 * begin at current position
 *
 * @param stream -- ICharStream realization
 *
 * @return one of tokens in union type
 */
token::token_union read_token(ICharStream &stream) {
  static auto [op, kw] = impl::initTries();
  auto opPoint = op.begin();
  auto kwPoint = kw.begin();
  size_t begin = stream.position();

  char c;
  stream >> c;
  if (opPoint->canGoTo(c)) {
    while (opPoint->canGoTo(c) && !opPoint->isTerm()) {
      opPoint->goTo(c);
      if (stream.eof())
        break;
      stream >> c;
    }
    stream << c;

    if (!opPoint->isTerm())
      throw error::UndefinedOperatorError(stream.position());

    auto opType = opPoint->get();
    return {token::Operation(opType, begin, stream.position())};
  }
  auto kwPointBegin = kw.begin();
  std::string buffer = "";

  while (!stream.eof() && !isSpace(c) && !opPoint->canGoTo(c)) {
    if (*kwPoint != *kwPointBegin && kwPoint->canGoTo(c))
      kwPoint->goTo(c);
    buffer.push_back(c);
    stream >> c;
  }

  if (!stream.eof())
    stream << c;

  if (kwPoint->isTerm())
    return {token::Keyword(kwPoint->get(), begin, stream.position())};
  return {token::Name(buffer, begin, stream.position())};
}

token::tokens_list token::Tokenizer::parse(ICharStream &stream) {
  char c;
  token::tokens_list result;
  while (!stream.eof()) {
    stream >> c;

		if(c == '"') {
			size_t commentBegin = stream.position();
			do {
				stream >> c;
			} while(!stream.eof() && (c != '"'));
			if(c != '"' || stream.eof()) 
				throw error::CommentNotClosedError(commentBegin);
			continue;
		}

    if (isspace(c))
      continue;

    stream << c;
    result.push_back(read_token(stream));
  }

  return result;
}
