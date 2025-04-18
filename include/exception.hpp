#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include "Tokenizer.hpp"
#include "utils.hpp"
#include <exception>
#include <string>

namespace error {
class IPositionError : public std::exception {
public:
  virtual size_t position() const = 0;
};

class PositionErrorBase : public IPositionError {
  size_t position_;

public:
  PositionErrorBase(size_t position) : position_(position) {}
  size_t position() const override { return position_; }
};

class UndefinedOperatorError : public PositionErrorBase {
  size_t position_;
  std::string message_;

public:
  UndefinedOperatorError(size_t position) : PositionErrorBase(position) {}
  const char *what() const noexcept override {
    return "Undefined operator, names with operators at begin not allowed";
  }
};

class CommentNotClosedError : public PositionErrorBase {
public:
  CommentNotClosedError(size_t position) : PositionErrorBase(position) {}
  const char *what() const noexcept override { return "Comment not closed"; }
};

class BordersError : public std::exception {
  size_t begin_, end_;

public:
  BordersError(size_t begin, size_t end) : begin_(begin), end_(end) {}

  const char *what() const noexcept override {
    static std::string message;
    message =
        (strfast() << "begin(" << begin_ << ") of token must be less then end("
                   << end_ << ")")
            .bump();
    return message.c_str();
  }
};

class ExpectedMismatchError : public PositionErrorBase {
  size_t position_;
  std::string expected_, given_;

public:
  ExpectedMismatchError(size_t position, std::string expected,
                        std::string given)
      : PositionErrorBase(position), expected_(expected), given_(given) {}

  const char *what() const noexcept override {
    static std::string message;
    message = (strfast() << "Semantic error: expected [" << expected_
                         << "], but given: [" << given_ << "]")
                  .bump();

    return message.c_str();
  }
};

class SemanticError : public PositionErrorBase {
  std::string what_;

public:
  SemanticError(size_t position, std::string what)
      : PositionErrorBase(position), what_(what) {}
  const char *what() const noexcept override { return what_.c_str(); }
};

class RedefinitionError : public PositionErrorBase {
  std::string name_;

public:
  RedefinitionError(token::token_union &token)
      : PositionErrorBase(token.begin()), name_(token.getName()) {}
  const char *what() const noexcept override {
    static std::string message;
    message = (strfast() << "Redefinition of mt: [" << name_ << "]").bump();

    return message.c_str();
  }
};

class ClosingTokenBeforeOpenTokenError : public PositionErrorBase {
  std::string tokenName_;

public:
  ClosingTokenBeforeOpenTokenError(token::token_union &token)
      : PositionErrorBase(token.begin()), tokenName_(token.toString()) {}

  const char *what() const noexcept override {
    static std::string message;
    message =
        (strfast() << "Closing [" << tokenName_ << "] occured before open ones")
            .bump();

    return message.c_str();
  }
};

class ClosingTokenNotFound : public PositionErrorBase {
  std::string tokenName_;

public:
  ClosingTokenNotFound(token::token_union &token)
      : PositionErrorBase(token.end()), tokenName_(token.toString()) {}

  const char *what() const noexcept override {
		static std::string message;
		message = (strfast() << "Closing token for [" << tokenName_ << "]").bump();
	
		return message.c_str();
	}
};

class UnexpectedTokenError : public PositionErrorBase {
  std::string tokenName_;

public:
  UnexpectedTokenError(token::token_union &token)
      : PositionErrorBase(token.begin()), tokenName_(token.toString()) {}

  const char *what() const noexcept override {
    static std::string message =
        (strfast() << "Token [" << tokenName_ << "] unexpected in this context")
            .bump();

    return message.c_str();
  }
};

} // namespace error

#endif // !EXCEPTIONS_HPP_
