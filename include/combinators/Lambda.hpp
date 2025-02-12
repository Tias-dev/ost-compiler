#ifndef LAMBDA_COMBINATOR_HPP_
#define LAMBDA_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
#include <functional>
namespace combinator {
template <typename ...Args>
using PLambda = std::function<Result<Args...>(ICharStream &)>;

template <typename ...Args>
using RLambda = std::function<void(Result<Args...> &, ICharStream&)>;

template <typename ...Args>
class Lambda : public ICombinator<Args...>{
  PLambda<Args...> lambdaParse_;
  RLambda<Args...> lambdaRevert_;
public:
  Lambda(PLambda<Args...> lambdaParse, RLambda<Args...> lambdaRevert) : lambdaParse_(lambdaParse), lambdaRevert_(lambdaRevert) {};
  Result<Args...> parse(ICharStream & stream) override {
    Result<Args...> res = lambdaParse_(stream);
		res.parser = this;
		return res;
  }

  void revert(Result<Args...> & res, ICharStream & stream) override {
		if(res.status == ResultStatus::Success) 
			lambdaRevert_(res, stream);
  }

};

} // !combinator;

#endif // !LAMBDA_COMBINATOR_HPP_
