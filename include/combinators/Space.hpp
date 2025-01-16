#ifndef SPACE_COMBINATOR_HPP_
#define SPACE_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
class SpaceCombinator : public ICombinator<> {
public:
  Result<> parse(ICharStream & stream) override;
};

namespace combinator {
  using space = SpaceCombinator;
} // !combinator;

#endif // !SPACE_COMBINATOR_HPP_
