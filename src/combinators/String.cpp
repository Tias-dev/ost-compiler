#include "combinators/String.hpp"
#include "Combinator.hpp"
#include <memory>
#include <sstream>

namespace combinator {

Result<> String::parse(ICharStream &stream) {
  Result<> result;
  size_t i = 0;
  char c;
  while (i < pattern_.size()) {
    stream >> c;
    if (c != pattern_[i]) {
      break;
    }
    ++i;
  }

  if (i < pattern_.size()) { // failure
    stream << c;
    for (size_t j = i - 1; j != (size_t)(-1); --j) {
      stream << pattern_[j];
    }

    result.status = ResultStatus::Failure;
    result.errorMessage =
        (std::stringstream()
         << "Letter mismatch at " << i << "index\n"
         << "Expected: " << pattern_.substr(0, i) << "\nGiven: "
         << pattern_.substr(0, i - 1 < pattern_.size() ? i - 1 : 0) << c
         << '\n')
            .str();
    return result;
  }

  result.status = ResultStatus::Success;
  result.parsedLen = pattern_.size();
  return result;
}

void String::revert(Result<> &res, ICharStream &stream) { stream << pattern_; }

ptr<> String::create(std::string &pattern) {
  return std::make_shared<String>(pattern);
}

} // namespace combinator
