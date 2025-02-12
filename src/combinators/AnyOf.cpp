#include "combinators/AnyOf.hpp"
#include "CharStream.hpp"
#include "Combinator.hpp"
#include "combinators/Space.hpp"
#include <iostream>
#include <memory>
#include <sstream>

namespace combinator {
Result<char> AnyOf::parse(ICharStream & stream) {
  Result<char> result;
  char c;
  stream >> c;
  for(char elem : letters_) {
    if(elem == c) {
      result.status = ResultStatus::Success;
      result.parsedLen = 1;
			result.data = {c};
      lastChar = c;
      return result;
    }
  }

  stream << c;
  result.status = ResultStatus::Failure;
  result.errorMessage = (
      std::stringstream()
      << "The letter: " << c <<
      " not in " << letters_ << " set\n"
      ).str();

  return result;
}
void AnyOf::revert(Result<char> & res, ICharStream & stream) {
  if(res.status != ResultStatus::Success) {
    return;
  }
	
	auto [c] = res.data.value();
  stream << c;
}

ptr<char> AnyOf::create(std::string &letters) {
	return std::make_shared<AnyOf>(letters);
}
} // !combinator;
