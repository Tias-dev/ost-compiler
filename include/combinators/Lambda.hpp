#ifndef LAMBDA_COMBINATOR_HPP_
#define LAMBDA_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
#include <functional>
namespace combinator {
template <typename ...Args>
using PLambda = std::function<Result<Args...>(ICharStream &)>;

template <typename ...Args>
class Lambda : public ICombinator<Args...>{
  PLambda<Args...> lambda_;
public:
  Lambda(PLambda<Args...> lambda) : lambda_(lambda) {};
  Result<Args...> parse(ICharStream & stream) override {
    return lambda_(stream);
  }
};

} // !combinator;

#endif // !LAMBDA_COMBINATOR_HPP_
