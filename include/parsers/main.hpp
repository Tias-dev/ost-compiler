#ifndef PARSERS_HPP_
#define PARSERS_HPP_

#include "combinators/AnyOf.hpp"
#include "combinators/Space.hpp"
#include "combinators/String.hpp"
#include "combinators/utils.hpp"
#include <string>
namespace combinator {
inline std::string kwAlphabet =
	"abcdefghijklmnopqrstuyxwz"
	"ABCDEFGHIJKLMNOPQRSTUYXWZ"
	"абвгдеёжзийклмнопрстуфхцчшщьыъэюя"
	"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЬЫЪЭЮЯ"
	"0123456789";

inline auto name = to_string(many(AnyOf::create(kwAlphabet)));
inline auto space = opt(many(Space::create()));
inline auto terminator = String::create(";");
inline auto mt = space >> String::create("MT") >> space >> name << terminator;
} // namespace combinator 


#endif // !PARSERS_HPP_
