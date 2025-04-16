#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include <exception>
#include <sstream>
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
    static std::string message =
        (std::stringstream()
         << "begin(" << begin_ << ") of token must be less then end(" << end_
         << ")\n")
            .str();
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
    static std::string message =
        (std::stringstream() << "Semantic error: expected [" << expected_
                             << "], but given: [" << given_ << "]")
            .str();

    return message.c_str();
  }
};
} // namespace error

#endif // !EXCEPTIONS_HPP_
