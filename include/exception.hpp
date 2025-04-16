#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include <exception>
#include <sstream>
#include <string>
namespace error {

	class IParseError : public std::exception {
		public:
		virtual size_t position() const = 0;
	};
	
	class ParseErrorBase : public IParseError {
		size_t position_;
	public:
		ParseErrorBase(size_t position) : position_(position) {}
		size_t position() const override {return position_;}
	};

	class UndefinedOperatorError : public ParseErrorBase {
		size_t position_;
		std::string message_;
		public:
		UndefinedOperatorError(size_t position) : ParseErrorBase(position) {}
		const char * what() const noexcept override {
			return "Undefined operator, names with operators at begin not allowed";
		}
	};

	class CommentNotClosedError : public ParseErrorBase {
	public:
		CommentNotClosedError(size_t position) : ParseErrorBase(position) {}
		const char * what() const noexcept override {
			return "Comment not closed";
		}
	};

	class BordersError : public std::exception {
		size_t begin_, end_;
	public:
		BordersError(size_t begin, size_t end)
			: begin_(begin), end_(end) {}

		const char * what() const noexcept override {
			static std::string message;
			message = (std::stringstream() << "begin(" << begin_ << ") of token must be less then end(" << end_ << ")\n").str();
			return message.c_str();
		}
	};
} // namespace error

#endif // !EXCEPTIONS_HPP_
