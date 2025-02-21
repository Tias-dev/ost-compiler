#ifndef COMBINATOR_HPP_
#define COMBINATOR_HPP_

#include "CharStream.hpp"
#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace combinator {
enum class ResultStatus { Success, Failure };

template <typename... Args> class ICombinator;
template <typename... Args> class Result;

template <typename... Args> using ptr = std::shared_ptr<ICombinator<Args...>>;
template <typename... Args> using ptr_res = std::shared_ptr<Result<Args...>>;

template <typename... Args> class Result {
protected:
  ResultStatus status_;
  std::string errorMessage_;
  size_t parsedLen_;
  std::optional<std::tuple<Args...>> data_;
public:
  Result(ResultStatus _status,
         std::optional<std::tuple<Args...>> _data = std::nullopt,
         size_t _parsedLen = 0, std::string _errorMessage = "")
      : status_(_status), parsedLen_(_parsedLen), data_(_data),
        errorMessage_(_errorMessage) {}

	Result(std::tuple<Args...> data, size_t parsedLen) : status_(ResultStatus::Success), data_({data}), parsedLen_(parsedLen), errorMessage_("") {}
	Result(std::string errorMessage) : status_(ResultStatus::Failure), errorMessage_(errorMessage), data_(std::nullopt), parsedLen_(0) {}

	const ResultStatus & status() const {return status_;}
	const std::string & errorMessage() const {return errorMessage_;}
	size_t parsedLen() const {return parsedLen_;}
	const std::optional<std::tuple<Args...>> & data() const {return data_;}

	virtual void revert(ICharStream & stream) = 0;
};

template <typename... Args> class ICombinator {
public:
  virtual ptr_res<Args...> parse(ICharStream &stream) = 0;
};

} // namespace combinator

namespace cmb = combinator;

#endif // !COMBINATOR_HPP_
