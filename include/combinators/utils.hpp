#ifndef UTILS_HPP_
#define UTILS_HPP_
#include "Combinator.hpp"
#include "combinators/Lambda.hpp"
#include <variant>

namespace combinator {
/**
 * @brief Consume first parser and if it succeed the second
 * if the second is failed first not reverting
 *
 * @tparam ...Args1
 * @tparam ...Args2
 * @param p1
 * @param p2
 *
 * @return 
 */
template <typename... Args1, typename... Args2>
ptr<Args2...> operator>>(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream &stream) {
    Result<Args1...> res1 = p1->parse(stream);
    Result<Args2...> res2;
    if (res1.status == ResultStatus::Failure) {
      res2.status = ResultStatus::Failure;
      res2.errorMessage = res1.errorMessage;
      return res2;
    }

    res2 = p2->parse(stream);
    if (res2.status == ResultStatus::Failure) {
      return res2;
    }

    res2.parsedLen += res1.parsedLen;
    return res2;
  };

  auto funcRevert = [p1, p2](Result<Args2...> & res, ICharStream &stream) {
    p2->revert(res, stream);
  };

  return std::make_shared<Lambda<Args2...>>(func, funcRevert);
}

template <typename... Args1, typename... Args2>
ptr<Args1...> operator<<(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto funcParse = [p1, p2](ICharStream &stream) {
    Result<Args1...> res1 = p1->parse(stream);
    if (res1.status == ResultStatus::Failure)
      return res1;

    Result<Args2...> res2 = p2->parse(stream);
    if (res2.status == ResultStatus::Failure) {
      res1.status = ResultStatus::Failure;
      res1.errorMessage = res2.errorMessage;
      return res1;
    }
    res1.parsedLen += res2.parsedLen;
    return res1;
  };

  auto funcRevert = [p1, p2](Result<Args1...> &res, ICharStream &stream) {
    p1->revert(res, stream);
  };

  return std::make_shared<Lambda<Args1...>>(funcParse, funcRevert);
}

template <typename... Args1, typename... Args2>
ptr<Args1..., Args2...> operator&&(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream &stream) {
    Result<Args1..., Args2...> res;
    Result<Args1...> res1 = p1->parse(stream);
    if (res1.status == ResultStatus::Failure) {
      res.status = ResultStatus::Failure;
      res.errorMessage = res1.errorMessage;
      return res;
    }

    Result<Args2...> res2 = p2->parse(stream);
    if (res2.status == ResultStatus::Failure) {
      p1->revert(stream);
      res.status = ResultStatus::Failure;
      res.errorMessage = res2.errorMessage;
      return res;
    }

    res.data = std::tuple_cat(res1.data.value(), res2.data.value());
    res.parsedLen = res1.parsedLen + res2.parsedLen;
    return res;
  };

  auto funcRevert = [p1, p2](Result<Args1..., Args2...> & resICharStream &stream) {
    p1->revert(stream);
    p2->revert(stream);
  };

  return std::make_shared<Lambda<Args1..., Args2...>>(func, funcRevert);
}

template <typename... Args1, typename... Args2>
ptr<std::variant<Result<Args1...>, Result<Args2...>>>
operator||(ptr<Args1...> p1, ptr<Args2...> p2) {
  using ret_type = std::variant<Result<Args1...>, Result<Args2...>>;
  auto funcParse = [p1, p2](ICharStream &stream) {
    Result<ret_type> res;
    Result<Args1...> res1 = p1->parse(stream);
    if (res1.status == ResultStatus::Success) {
      res.data = ret_type{res1};
      res.parsedLen = res1.parsedLen;
      return res;
    }

    p1->revert(stream);
    Result<Args2...> res2 = p2->parse(stream);
    if (res2.status == ResultStatus::Success) {
      res.data = ret_type{res2};
      res.parsedLen = res2.parsedLen;
      return res;
    }
    p2->revert(stream);
    res.status = ResultStatus::Failure;
    return res;
  };

  auto funcRevert = [p1, p2](Result<ret_type> &res, ICharStream &stream) {
    if (res.status != ResultStatus::Success)
      return;
    struct Reverter {
      ptr<Args1...> p1;
      ptr<Args2...> p2;
			ICharStream & stream;

      Reverter(ptr<Args1...> p1_, ptr<Args2...> p2_, ICharStream & stream_) : p1(p1_), p2(p2_), stream(stream_) {};

      void operator()(Result<Args1...> &res) { this->p1->revert(res, stream); }
      void operator()(Result<Args2...> &res) { this->p2->revert(res, stream); }
    };

    std::visit(Reverter{p1, p2}, res.data.value());
  };

  return std::make_shared<Lambda<ret_type>>(funcParse, funcRevert);
}

  /**
   * @brief create combinator which parse as many times given parser as it can
   *
   * @param ptr source parser
   *
   * @return new parser
   */
template <typename... Args>
ptr<std::list<Result<Args...>>> many(ptr<Args...> p) {
  using ret_type = std::list<Result<Args...>>;
  auto funcParse = [p](ICharStream &stream) {
    ret_type values{};
    size_t parsedLen = 0;
    Result<Args...> res = p->parse(stream);
    while (res.status == ResultStatus::Success) {
      values.push_back(res);
      parsedLen += res.parsedLen;
      res = p->parse(stream);
    }
    return Result<ret_type>{.status = ResultStatus::Success,
                            .parsedLen = parsedLen,
                            .data = values};
  };

  auto funcRevert = [p](Result<ret_type> &res, ICharStream &stream) {
    if (res.status != ResultStatus::Success)
      return;
    for (auto &elem : std::get<0>(res.data.value())) {
      p->revert(elem, stream);
    }
  };

  return std::make_shared<Lambda<ret_type>>(funcParse, funcRevert);
}
} // namespace combinator
	
#endif // !UTILS_HPP_
