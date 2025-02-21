#include "combinators/AnyOf.hpp"
#include "CharStream.hpp"
#include "Combinator.hpp"
#include <memory>
#include <optional>
#include <sstream>

namespace combinator {
ptr_res<char> AnyOf::parse(ICharStream &stream) {
  char c;
  stream >> c;
  for (char elem : letters_) {
    if (elem == c)
      return AnyOfResult::createSuccess(c);
  }

  stream << c;
  return AnyOfResult::createFailure((std::stringstream()
                                           << "The letter: " << c << " not in {"
                                           << letters_ << "} set\n")
                                              .str());
}

ptr<char> AnyOf::create(std::string &letters) {
  return std::make_shared<AnyOf>(letters);
}

ptr_res<char> AnyOfResult::createSuccess(char c) {
  return std::make_shared<AnyOfResult>(
      ResultStatus::Success, std::optional<std::tuple<char>>{c}, 1, "");
}

ptr_res<char> AnyOfResult::createFailure(std::string errorMessage) {
  return std::make_shared<AnyOfResult>(ResultStatus::Failure,
                                       std::nullopt, 0, errorMessage);
}

void AnyOfResult::revert(ICharStream & stream) {
	if(data_.has_value()) {
		stream << std::get<0>(data_.value());
	}
}
} // namespace combinator
