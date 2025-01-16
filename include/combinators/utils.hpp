#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "Combinator.hpp"
#include <list>

namespace combinator {
  /**
   * @brief create combinator which parse as many times given parser as it can
   *
   * @param ptr source parser
   *
   * @return new parser
   */
  template <typename ...Args>
  ptr<std::list<Result<Args...>>> many(ptr<Args...>);
} // !combinator;

#endif // !UTILS_HPP_
