#ifndef ANY_OF_COMBINATOR_HPP_
#define ANY_OF_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
namespace combinator {
class AnyOf : public ICombinator<char> {
  std::string letters_;
  char lastChar = 0;
public:
  AnyOf(std::string &letters) : letters_(letters) {}
  Result<char> parse(ICharStream & stream) override;
  void revert(Result<char> & res, ICharStream & stream) override;

	static ptr<char> create(std::string & letters);
};


} // !combinator;
#endif // !ANY_OF_COMBINATOR_HPP_
