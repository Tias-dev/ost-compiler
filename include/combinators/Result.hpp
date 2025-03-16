#ifndef RESULT_HPP_
#define RESULT_HPP_
#include "CharStream.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
namespace combinator {
enum class ResultStatus { Success, Failure };
template <typename ...Args>
class IResult {
	public:
	virtual const ResultStatus & status() const = 0;
	virtual const std::string & errorMessage() const = 0;
	virtual size_t parsedLen() const = 0;
	virtual const std::optional<std::tuple<Args...>> & data() const = 0;

	virtual void revert(ICharStream & stream) = 0;
};

template <typename... Args> using ptr_res = std::shared_ptr<IResult<Args...>>;

template <typename... Args> class ResultBase : public IResult<Args...> {
protected:
  ResultStatus status_;
  std::string errorMessage_;
  size_t parsedLen_;
  std::optional<std::tuple<Args...>> data_;
public:
  ResultBase(ResultStatus _status,
         std::optional<std::tuple<Args...>> _data = std::nullopt,
         size_t _parsedLen = 0, std::string _errorMessage = "")
      : status_(_status), parsedLen_(_parsedLen), data_(_data),
        errorMessage_(_errorMessage) {}

	ResultBase(std::tuple<Args...> data, size_t parsedLen) : status_(ResultStatus::Success), data_({data}), parsedLen_(parsedLen), errorMessage_("") {}
	ResultBase(std::optional<std::tuple<Args...>> data, size_t parsedLen) : status_(ResultStatus::Success), data_(data), parsedLen_(parsedLen), errorMessage_("") {}
	ResultBase(std::string errorMessage) : status_(ResultStatus::Failure), errorMessage_(errorMessage), data_(std::nullopt), parsedLen_(0) {}

	const ResultStatus & status() const override {return status_;}
	const std::string & errorMessage() const override {return errorMessage_;}
	size_t parsedLen() const override {return parsedLen_;}
	const std::optional<std::tuple<Args...>> & data() const override {return data_;}

	virtual void revert(ICharStream & stream) override {};
};
} // namespace combinator
#endif // !RESULT_HPP_
