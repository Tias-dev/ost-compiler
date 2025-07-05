#include "FilePosition.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>

FileRoller::FileRoller(std::shared_ptr<std::string> fileName)
    : fileName_(fileName) {
  std::ifstream file(*fileName);
  if (!file.is_open())
    throw std::runtime_error("Given file name can't be opened");

  size_t index = 0;
  while (!file.eof()) {
    char c = file.get();
    if (c == '\n')
      newLines_.push_back(index);
    index += 1;
  }
}

FileRoller::Position FileRoller::convert(size_t index) const {
  size_t i = 0;
  while (newLines_[i] < index)
    ++i;

  return Position{.row = i + 1,
                  .column = (i > 0 ? index - newLines_[i - 1] : index)};
}

const std::shared_ptr<std::string> FilePosition::defaultFName_ =
    std::make_shared<std::string>("Not set");

std::string FilePosition::to_string() const {
  return strfast() << *fileName_ << ":" << row_ << ":" << column_;
}

FilePosition FilePosition::from_string(const std::string &s) {
  std::string fileName;
  size_t row, col;
  size_t i = 0, j = 0;
  while (i < s.size() && s[i] != ':')
    ++i;
  if (i == s.size())
    throw std::invalid_argument(
        strfast() << "FilePosition: usage: fileName:row:col, given: " << s);

  fileName = s.substr(j, i - j);
  ++i, j = i;

  while (i < s.size() && s[i] != ':')
    ++i;
  if (i == s.size())
    throw std::invalid_argument(
        strfast() << "FilePosition: usage: fileName:row:col, given: " << s);

  row = atoll(s.substr(j, i - j).c_str());
  ++i, j = i;
  col = atoll(s.substr(j).c_str());

  return FilePosition{std::make_shared<std::string>(fileName), row, col};
}
