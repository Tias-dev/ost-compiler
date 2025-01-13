#ifndef COMBINATOR_HPP_
#define COMBINATOR_HPP_

#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include "CharStream.hpp"

enum class ResultStatus {
  Success,
  Failure
};

template <typename ...Args>
struct Result {
  ResultStatus status;
  std::string errorMessage = "";
  size_t parsedLen = 0;
  std::optional<std::tuple<Args...>> data = std::nullopt;
};

template <typename ...Args>
class ICombinator {
public:
  virtual Result<Args...> parse(ICharStream & stream) = 0;
};

template <typename ...Args>
using ptr = std::shared_ptr<ICombinator<Args...>>;

template <typename ...Args1, typename ...Args2>
ptr<Args1..., Args2...> operator>>(ptr<Args1...> p1, ptr<Args2...> p2);

#endif // !COMBINATOR_HPP_
