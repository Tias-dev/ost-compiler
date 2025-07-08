#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include "FilePosition.hpp"
#include "Tokenizer.hpp"
#include "utils.hpp"
#include <exception>
#include <stdexcept>
#include <string>

namespace error {
class IPositionError : public std::exception {
public:
	virtual const char * what() const noexcept = 0;
  virtual const FilePosition & position() const = 0;
};

class PositionErrorBase : public IPositionError {
  FilePosition position_;
public:
  PositionErrorBase(const FilePosition & position) : position_(position) {}
  const FilePosition & position() const override { return position_; }
};

class UndefinedOperatorError : public PositionErrorBase {
  size_t position_;
  std::string message_;

public:
  UndefinedOperatorError(const FilePosition & position) : PositionErrorBase(position) {}
  const char *what() const noexcept override {
    return "Undefined operator, names with operators at begin not allowed";
  }
};

class CommentNotClosedError : public PositionErrorBase {
public:
  CommentNotClosedError(const FilePosition & position) : PositionErrorBase(position) {}
  const char *what() const noexcept override { return "Comment not closed"; }
};

class BordersError : public std::exception {
  FilePosition begin_, end_;

public:
  BordersError(const FilePosition & begin, const FilePosition & end) : begin_(begin), end_(end) {}

  const char *what() const noexcept override {
    static std::string message;
    message =
        (strfast() << "begin(" << begin_.to_string() << ") of token must be less then end("
                   << end_.to_string() << ")");
    return message.c_str();
  }
};

class ExpectedMismatchError : public PositionErrorBase {
  std::string expected_, given_;

public:
  ExpectedMismatchError(const FilePosition & position, std::string expected,
                        std::string given)
      : PositionErrorBase(position), expected_(expected), given_(given) {}

  const char *what() const noexcept override {
    static std::string message;
    message = (strfast() << "Expected mismatch error: expected [" << expected_
                         << "], but given: [" << given_ << "]");

    return message.c_str();
  }
};

class SemanticError : public PositionErrorBase {
  std::string what_;

public:
  SemanticError(const FilePosition & position, std::string what)
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

class UnexpectedFileEnd : public std::exception {
	std::string expected_;
public:
	UnexpectedFileEnd(std::string expected = "") : expected_(expected) {}

	const char * what() const noexcept override {
		static std::string message;
		message = (strfast() << "Unexpected end of file" << (expected_.size() > 0 ? ". Expected: " + expected_ : "")).bump();
		return message.c_str();
	}
};
} // namespace error

namespace tu4run {
template <typename CharT>
class Line;
} // namespace tu4run
	
namespace error {
template <typename CharT>
std::ostream & operator<<(std::ostream & os, const tu4run::Line<CharT> & line);

template <typename CharT = char>
class Tu4RunError : public std::runtime_error {
	const tu4run::Line<CharT> & line_;
	std::string detail_;
public:
	Tu4RunError(const tu4run::Line<CharT> & line, std::string detail)
		: line_(line), detail_(detail), std::runtime_error(detail) {}

	const char * what() const noexcept override {
		static std::string message;
		message = (strfast() << "Line state:\n" << line_ << "\nError: " << detail_);

		return message.c_str();
	}
};

} // namespace error

#endif // !EXCEPTIONS_HPP_
