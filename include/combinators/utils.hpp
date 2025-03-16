#ifndef UTILS_HPP_
#define UTILS_HPP_
#include "CharStream.hpp"
#include "Combinator.hpp"
#include "combinators/Combining.hpp"
#include "combinators/Result.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <utility>
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
  auto func = [p1, p2](ICharStream &stream) -> ptr_res<Args2...> {
    ptr_res<Args1...> res1 = p1->parse(stream);
    if (res1->status() == ResultStatus::Failure) {
      return std::make_shared<ResultBase<Args2...>>(res1->errorMessage());
    }

    ptr_res<Args2...> res2 = p2->parse(stream);
    if (res2->status() == ResultStatus::Failure) {
      res1->revert(stream);
      return res2;
    }

    ptr_res<Args2...> result =
        std::make_shared<CombinedResult<ResultBase<Args2...>, IResult<Args1...>,
                                        IResult<Args2...>>>(
            ResultBase<Args2...>(res2->data().value(),
                                 res2->parsedLen() + res1->parsedLen()),
            std::tuple{res1, res2});
    return result;
  };

  return std::make_shared<Lambda<Args2...>>(func);
}

template <typename... Args1, typename... Args2>
ptr<Args1...> operator<<(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream &stream) -> ptr_res<Args1...> {
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

    ptr_res<Args1...> result =
        std::make_shared<CombinedResult<ResultBase<Args1...>, IResult<Args1...>,
                                        IResult<Args2...>>>(
            ResultBase<Args1...>(res1->data().value(),
                                 res1->parsedLen() + res2->parsedLen()),
            std::tuple{res1, res2});
    return result;
  };

  return std::make_shared<Lambda<Args1...>>(func);
}

template <typename... Args1, typename... Args2>
ptr<Args1..., Args2...> operator&&(ptr<Args1...> p1, ptr<Args2...> p2) {
  auto func = [p1, p2](ICharStream &stream) -> ptr_res<Args1..., Args2...> {
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
    return std::make_shared<CombinedResult<
        ResultBase<Args1..., Args2...>, IResult<Args1...>, IResult<Args2...>>>(
        ResultBase(data, res1->parsedLen() + res2->parsedLen()),
        std::tuple{res1, res2});
  };

  return std::make_shared<Lambda<Args1..., Args2...>>(func);
}

enum class SuccessResult { first, second };

template <typename... Results>
struct VariantResult : public ResultBase<std::shared_ptr<Results>...> {
  using base_type = ResultBase<std::shared_ptr<Results>...>;

  VariantResult(std::tuple<std::shared_ptr<Results>...> results,
                size_t parsedLen)
      : base_type(results, parsedLen) {}
  VariantResult(std::string errorMessage) : base_type(errorMessage) {}

  void revert(ICharStream &stream) override {
    if (this->status_ != ResultStatus::Success)
      return;

    [&stream]<size_t... Idx>(auto &results, std::index_sequence<Idx...>) {
      (std::get<Idx>(results)->revert(stream), ...);
    }(this->data().value(), std::make_index_sequence<sizeof...(Results)>{});
  }
};

template <typename... Args1, typename... Args2>
ptr<ptr_res<Args1...>, ptr_res<Args2...>> operator||(ptr<Args1...> p1,
                                                     ptr<Args2...> p2) {
  using ret_type = VariantResult<IResult<Args1...>, IResult<Args2...>>;

  auto funcParse = [p1, p2](ICharStream &stream) -> std::shared_ptr<ret_type> {
    ptr_res<Args1...> res1 = p1->parse(stream);
    if (res1->status() == ResultStatus::Success) {
      return std::make_shared<ret_type>(
          std::tuple{res1, std::make_shared<ResultBase<Args2...>>("")},
          res1->parsedLen());
    }

    res1->revert(stream);
    ptr_res<Args2...> res2 = p2->parse(stream);
    if (res2->status() == ResultStatus::Success) {
      return std::make_shared<ret_type>(std::tuple{res1, res2},
                                        res2->parsedLen());
    }
    res2->revert(stream);
    return std::make_shared<ret_type>(res1->errorMessage() + "\n---------\n" +
                                      res2->errorMessage());
  };

  return std::make_shared<Lambda<ptr_res<Args1...>, ptr_res<Args2...>>>(
      funcParse);
}

template <typename... Args> class LambdaResult : public ResultBase<Args...> {
  using revert_func_type_ =
      std::function<void(ICharStream &, std::tuple<Args...> &)>;
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
  auto funcParse = [p](ICharStream &stream) -> ptr_res<ret_type> {
    ret_type values{};
    size_t parsedLen = 0;
    ptr_res<Args...> res = p->parse(stream);
    while (res->status() == ResultStatus::Success) {
      values.push_back(res);
      parsedLen += res->parsedLen();
      res = p->parse(stream);
    }
    return std::make_shared<LambdaResult<ret_type>>(
        [](ICharStream &stream, std::tuple<ret_type> &data) {
          for (auto &result : std::get<0>(data)) {
            result->revert(stream);
          }
        },
        std::tuple{values}, parsedLen);
  };

  return std::make_shared<Lambda<ret_type>>(funcParse);
}

/**
 * @brief Optional parser, if parsedLen 0 is also Succeess
 *
 * @tparam ...Args
 * @param p
 *
 * @return optional parser
 */
template <typename... Args> ptr<Args...> opt(ptr<Args...> p) {
  auto func = [p](ICharStream &stream) -> ptr_res<Args...> {
    auto res = p->parse(stream);
    if (res->parsedLen() > 0)
      return res;
    return std::make_shared<ResultBase<Args...>>(std::nullopt, 0);
  };

  return std::make_shared<Lambda<Args...>>(func);
}

inline ptr<std::string> to_string(ptr<std::list<ptr_res<char>>> p) {
  auto func = [p](ICharStream &stream) -> ptr_res<std::string> {
    auto res = p->parse(stream);
    std::string s = "";
    if (res->status() == ResultStatus::Success) {
      for (auto &ptr_c : std::get<0>(res->data().value())) {
        s.push_back(std::get<0>(ptr_c->data().value_or('\0')));
      }

      return std::make_shared<LambdaResult<std::string>>(
          [](ICharStream &stream, std::tuple<std::string>& data) {
            stream << std::get<0>(data);
          },
					std::tuple{s}, res->parsedLen());
    }
		return std::make_shared<ResultBase<std::string>>(res->errorMessage());
  };

	return std::make_shared<Lambda<std::string>>(func);
}

} // namespace combinator

#endif // !UTILS_HPP_
