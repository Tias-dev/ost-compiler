#include "Combinator.hpp"
#include "CharStream.hpp"
#include "combinators/Lambda.hpp"
#include <memory>

namespace combinator {
template <typename ...Args1, typename ...Args2>
ptr<Args2...> operator>>(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream & stream) {
    Result<Args1...> res1 = p1->parse(stream);
    if(res1.status == ResultStatus::Failure) 
      return res1;
    
    Result<Args2...> res2 = p2->parse(stream);
    if(res2.status == ResultStatus::Failure) {
      p1->revert(stream);     
      return res2;
    }
    return res2;
  };

  return std::make_shared<Lambda<Args2...>>(func);
}

template <typename ...Args1, typename ...Args2>
ptr<Args1...> operator<<(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream & stream) {
    Result<Args1...> res1 = p1->parse(stream);
    if(res1.status == ResultStatus::Failure) 
      return res1;
    
    Result<Args2...> res2 = p2->parse(stream);
    if(res2.status == ResultStatus::Failure) {
      p1->revert(stream);     
      return res2;
    }
    return res1;
  };

  return std::make_shared<Lambda<Args1...>>(func);
}

template <typename ...Args1, typename ...Args2>
ptr<Args1..., Args2...> operator&&(ptr<Args1...> p1, ptr<Args2...> p2);

template <typename ...Args1, typename ...Args2>
std::pair<ptr<Args1...>, ptr<Args2...>> operator||(ptr<Args1...> p1, ptr<Args2...> p2);


} // !combinator;
