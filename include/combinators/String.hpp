#ifndef STRING_COMBINATOR_HPP_
#define STRING_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"

class StringCombinator : public ICombinator<std::string> {
private:
  std::string pattern_;
public:
  StringCombinator(const std::string & pattern) : pattern_(pattern) {}
  
  Result<std::string> parse(ICharStream & stream) override;
};

namespace combinator {
  using str = StringCombinator;
}

#endif // !STRING_COMBINATOR_HPP_
