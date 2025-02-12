#ifndef COMBINATOR_HPP_
#define COMBINATOR_HPP_

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include "CharStream.hpp"

namespace combinator {
enum class ResultStatus {
  Success,
  Failure
};


template <typename ...Args>
class ICombinator;

template <typename ...Args>
using ptr = std::shared_ptr<ICombinator<Args...>>;

template <typename ...Args>
struct Result {
  ResultStatus status;
  std::string errorMessage = "";
  size_t parsedLen = 0;
  std::optional<std::tuple<Args...>> data = std::nullopt;
	ICombinator<Args...>* parser = nullptr;
};

template <typename ...Args>
class ICombinator {
public:
  virtual Result<Args...> parse(ICharStream & stream) = 0;
  virtual void revert(Result<Args...> & res, ICharStream & stream) = 0;
};


} // !combinator;

namespace cmb = combinator;

#endif // !COMBINATOR_HPP_
