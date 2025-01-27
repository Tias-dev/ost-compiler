#ifndef SPACE_COMBINATOR_HPP_
#define SPACE_COMBINATOR_HPP_

#include "combinators/AnyOf.hpp"

namespace combinator {
class Space : public AnyOf {
public:
  Space(std::string spaceLetters = " \t\n\r") : AnyOf(spaceLetters) {};
};
} // !combinator;

#endif // !SPACE_COMBINATOR_HPP_
