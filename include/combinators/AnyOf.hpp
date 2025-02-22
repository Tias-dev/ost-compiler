#ifndef ANY_OF_COMBINATOR_HPP_
#define ANY_OF_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"
namespace combinator {
	class AnyOfResult;
class AnyOf : public ICombinator<char> {
  std::string letters_;
public:
  AnyOf(std::string &letters) : letters_(letters) {}

  virtual ptr_res<char> parse(ICharStream & stream) override;

	static ptr<char> create(std::string & letters);
};

class AnyOfResult : public ResultBase<char> {
	public:
	AnyOfResult(ResultStatus _status,
         std::optional<std::tuple<char>> _data = std::nullopt,
         size_t _parsedLen = 0, std::string _errorMessage = "")
	:	ResultBase<char>(_status, _data, _parsedLen, _errorMessage) {}

	void revert(ICharStream & stream) override;

	static ptr_res<char> createSuccess(char c);
	static ptr_res<char> createFailure(std::string errorMessage);
};
									

} // !combinator;
#endif // !ANY_OF_COMBINATOR_HPP_
