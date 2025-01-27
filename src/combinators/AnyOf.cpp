#include "combinators/AnyOf.hpp"
#include "CharStream.hpp"
#include "Combinator.hpp"
#include <sstream>

namespace combinator {
Result<> AnyOf::parse(ICharStream & stream) {
  Result<> result;
  char c;
  stream >> c;
  for(char elem : letters_) {
    if(elem == c) {
      result.status = ResultStatus::Success;
      result.parsedLen = 1;
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
} // !combinator;
