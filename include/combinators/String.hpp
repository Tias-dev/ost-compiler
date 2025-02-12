#ifndef STRING_COMBINATOR_HPP_
#define STRING_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"

namespace combinator {
class String : public ICombinator<> {
private:
  std::string pattern_;
public:
  String(const std::string & pattern) : pattern_(pattern) {}
  
  Result<> parse(ICharStream & stream) override;
  void revert(Result<> & res, ICharStream & stream) override;

	static ptr<> create(std::string & pattern);
};
} // !combinator;

#endif // !STRING_COMBINATOR_HPP_
