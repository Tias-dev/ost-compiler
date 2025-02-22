#ifndef COMBINATOR_HPP_
#define COMBINATOR_HPP_

#include "CharStream.hpp"
#include <memory>
#include "combinators/Result.hpp"

namespace combinator {
template <typename... Args> class ICombinator {
public:
  virtual ptr_res<Args...> parse(ICharStream &stream) = 0;
};

template <typename... Args> using ptr = std::shared_ptr<ICombinator<Args...>>;

} // namespace combinator

namespace cmb = combinator;

#endif // !COMBINATOR_HPP_
