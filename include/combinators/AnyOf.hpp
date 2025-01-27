#ifndef ANY_OF_COMBINATOR_HPP_
#define ANY_OF_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
namespace combinator {
class AnyOf : public ICombinator<> {
  std::string letters_;
public:
  AnyOf(std::string letters) : letters_(letters) {}
  Result<> parse(ICharStream & stream) override;
};


} // !combinator;
#endif // !ANY_OF_COMBINATOR_HPP_
