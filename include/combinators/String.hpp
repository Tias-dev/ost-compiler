#ifndef STRING_COMBINATOR_HPP_
#define STRING_COMBINATOR_HPP_

#include "CharStream.hpp"
#include "Combinator.hpp"

namespace combinator {

class StringResult;
class String : public ICombinator<> {
private:
  std::string pattern_;
public:
  String(const std::string & pattern) : pattern_(pattern) {}
  
  ptr_res<> parse(ICharStream & stream) override;

	static ptr<> create(std::string & pattern);
};

class StringResult : public Result<> {
	private:
		std::string pattern_;
	public:
	StringResult(std::string pattern, ResultStatus _status,
         std::optional<std::tuple<>> _data = std::nullopt,
         size_t _parsedLen = 0, std::string _errorMessage = "")
	:	Result<>(_status, _data, _parsedLen, _errorMessage), pattern_(pattern) {}

	void revert(ICharStream & stream) override;
	
};
} // !combinator;

#endif // !STRING_COMBINATOR_HPP_
