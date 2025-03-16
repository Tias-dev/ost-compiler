#ifndef OPERATORS_HPP_
#define OPERATORS_HPP_

#include <string>
enum class OperatorType {
	None,
	Comma,
	Equal,
	NotEqual,
	Question,
	Pow,
	OpenBracket,
	CloseBracket,
	SetLetter,
	DoutbleDot,
	Terminator,
	Lambda
};

OperatorType getOperatorType(const std::string &);
bool isOperator(const std::string &);

std::string to_string(OperatorType type);

#endif // !OPERATORS_HPP_
