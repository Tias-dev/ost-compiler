#ifndef UTILS_HPP_
#define UTILS_HPP_
#include "CharStream.hpp"
#include "Combinator.hpp"
#include "combinators/Combining.hpp"
#include "combinators/Result.hpp"
#include <functional>
#include <memory>
#include <optional>
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
  auto func = [p1, p2](ICharStream &stream)
	-> ptr_res<Args2...> {
    ptr_res<Args1...> res1 = p1->parse(stream);
    if (res1->status() == ResultStatus::Failure) {
      return std::make_shared<ResultBase<Args2...>>(res1->errorMessage());
    }

    ptr_res<Args2...> res2 = p2->parse(stream);
    if (res2->status() == ResultStatus::Failure) {
      res1->revert(stream);
      return res2;
    }

    ptr_res<Args2...> result = std::make_shared<
        CombinedResult<ResultBase<Args2...>, decltype(*res1), decltype(*res2)>>(
        *res2, {res1, res2});
    return result;
  };

  return std::make_shared<Lambda<Args2...>>(func);
}

template <typename... Args1, typename... Args2>
ptr<Args1...> operator<<(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream &stream)
	-> ptr_res<Args1...> {
    ptr_res<Args1...> res1 = p1->parse(stream);
    ptr_res<Args2...> res2;
    if (res1->status() == ResultStatus::Failure) {
      return res1;
    }

    res2 = p2->parse(stream);
    if (res2->status() == ResultStatus::Failure) {
      res1->revert(stream);
      return std::make_shared<ResultBase<Args1...>>(res2->errorMessage());
    }

    ptr_res<Args1...> result = std::make_shared<
        CombinedResult<ResultBase<Args1...>, IResult<Args1...>, IResult<Args2...>>>(
        ResultBase<Args1...>(res1->data().value(), res1->parsedLen()), std::tuple{res1, res2});
    return result;
  };

  return std::make_shared<Lambda<Args1...>>(func);
}

template <typename... Args1, typename... Args2>
ptr<Args1..., Args2...> operator&&(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream &stream) {
    ptr_res<Args1...> res1 = p1->parse(stream);
    if (res1->status() == ResultStatus::Failure) {
      return std::make_shared<ResultBase<Args1..., Args2...>>(
          res1->errorMessage());
    }

    ptr_res<Args2...> res2 = p2->parse(stream);
    if (res2->status() == ResultStatus::Failure) {
      res1->revert(stream);
      return std::make_shared<ResultBase<Args1..., Args2...>>(
          res2->errorMessage());
    }

    std::tuple<Args1..., Args2...> data =
        std::tuple_cat(res1->data().value(), res2->data().value());
    return std::make_shared<CombinedResult<ResultBase<Args1..., Args2...>,
                                           decltype(*res1), decltype(*res2)>>(
        ResultBase(data, res1->parsedLen() + res2->parsedLen()), {res1, res2});
  };

  return std::make_shared<Lambda<Args1..., Args2...>>(func);
}

template <typename... Args>
struct VariantResult : public ResultBase<std::variant<Args...>> {
  using base_type = ResultBase<std::variant<Args...>>;

  VariantResult(std::variant<Args...> data, size_t parsedLen)
      : base_type({data}, parsedLen) {}
  VariantResult(std::string errorMessage) : base_type(errorMessage) {}

  void revert(ICharStream &stream) override {
    if (this->status_ != ResultStatus::Success)
      return;

    std::visit([&stream](auto &result) { result->revert(stream); },
               this->data_.value());
  }
};

template <typename... Args1, typename... Args2>
ptr<std::variant<ptr_res<Args1...>, ptr_res<Args2...>>>
operator||(ptr<Args1...> p1, ptr<Args2...> p2) {
  using ret_type = std::variant<ptr_res<Args1...>, ptr_res<Args2...>>;
  auto funcParse = [p1, p2](ICharStream &stream) {
    ptr_res<Args1...> res1 = p1->parse(stream);
    if (res1->status() == ResultStatus::Success) {
      return std::make_shared<
          VariantResult<ptr_res<Args1...>, ptr_res<Args2...>>>({res1},
                                                               res1->parsedLen());
    }

    p1->revert(stream);
    ptr_res<Args2...> res2 = p2->parse(stream);
    if (res2->status() == ResultStatus::Success) {
      return std::make_shared<
          VariantResult<ptr_res<Args1...>, ptr_res<Args2...>>>({res2},
                                                               res2->parsedLen());
    }
    p2->revert(stream);
    return std::make_shared<ResultBase<ret_type>>(
        res1->errorMessage() + "\n---------\n" + res2->errorMessage());
  };

  return std::make_shared<Lambda<ret_type>>(funcParse);
}

template <typename... Args> class LambdaResult : ResultBase<Args...> {
  using revert_func_type_ =
      std::function<void(ICharStream &, std::optional<std::tuple<Args...>> &)>;
  revert_func_type_ revert_;

public:
  using revert_func_type = revert_func_type_;
  LambdaResult(revert_func_type revertFunc, std::tuple<Args...> data,
               size_t parsedLen)
      : ResultBase<Args...>(data, parsedLen), revert_(revertFunc) {}
  LambdaResult(std::string errorMessage) : ResultBase<Args...>(errorMessage) {}

  void revert(ICharStream &stream) override {
    if (this->status_ != ResultStatus::Success)
      return;

    revert_(stream, this->data_.value());
  }
};

/**
 * @brief create combinator which parse as many times given parser as it can
 *
 * @param ptr source parser
 *
 * @return new parser
 */
template <typename... Args>
ptr<std::list<ptr_res<Args...>>> many(ptr<Args...> p) {
  using ret_type = std::list<ptr_res<Args...>>;
  auto funcParse = [p](ICharStream &stream) {
    ret_type values{};
    size_t parsedLen = 0;
    ptr_res<Args...> res = p->parse(stream);
    while (res->status() == ResultStatus::Success) {
      values.push_back(res);
      parsedLen += res.parsedLen;
      res = p->parse(stream);
    }
    return std::make_shared<LambdaResult<ret_type>>(
        [](ICharStream &stream, std::tuple<ret_type> data) {
          for (auto &result : std::get<0>(data)) {
            result->revert(stream);
          }
        },
        {values}, parsedLen);
  };


  return std::make_shared<Lambda<ret_type>>(funcParse);
}
} // namespace combinator

#endif // !UTILS_HPP_
