#ifndef LEXEM_ANALYZER_HPP_
#define LEXEM_ANALYZER_HPP_

#include "CharStream.hpp"
#include "Lexem.hpp"
#include <list>
#include <ostream>

using LexemList = std::list<Lexem *>;

inline std::ostream & operator<<(std::ostream & os, const LexemList &lexems) {
	for(auto& elem : lexems) {
		os << *elem << '\n';
	}

	return os;
}

class LexemAnalyzer {
public:
  LexemList parse(ICharStream &stream);
};

#endif //! LEXEM_ANALYZER_HPP_
