#include "combinators/String.hpp"
#include "combinators/Combinator.hpp"
#include "combinators/Result.hpp"
#include <memory>
#include <optional>
#include <sstream>

namespace combinator {

ptr_res<> String::parse(ICharStream &stream) {
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

    auto result = std::make_shared<StringResult>(
        pattern_, ResultStatus::Failure, std::nullopt, 0,
        (std::stringstream()
         << "Letter mismatch at " << i << "index\n"
         << "Expected: " << pattern_.substr(0, i) << "\nGiven: "
         << pattern_.substr(0, i - 1 < pattern_.size() ? i - 1 : 0) << c
         << '\n')
            .str());
    return result;
  }

  auto result = std::make_shared<StringResult>(pattern_, ResultStatus::Success,
                                               std::tuple<>{}, pattern_.size(), "");
  return result;
}

ptr<> String::create(std::string &pattern) {
  return std::make_shared<String>(pattern);
}

void StringResult::revert(ICharStream & stream) {
	if(status() != ResultStatus::Success) 
		return;

	stream << pattern_;
};

} // namespace combinator
