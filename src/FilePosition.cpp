#include "FilePosition.hpp"

#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

#include "Bimap.hpp"
#include "utils.hpp"

FileRoller::FileRoller(std::shared_ptr<std::string> fileName)
    : fileName_(fileName) {
  std::ifstream file(*fileName);
  if (!file.is_open())
    throw std::runtime_error("Given file name can't be opened");

  SIZE_T index = 0;
  while (!file.eof()) {
    char c = file.get();
    if (c == '\n') newLines_.push_back(index);
    index += 1;
  }
}

FileRoller::Position FileRoller::convert(SIZE_T index) const {
  SIZE_T i = 0;
  while (newLines_[i] < index) ++i;

  return Position{.row = i + 1,
                  .column = (i > 0 ? index - newLines_[i - 1] : index)};
}

std::string FilePosition::to_string() const {
  return strfast() << fileCodesBimap_[code_] << ":" << row_ << ":" << column_;
}
bimap<std::string, SIZE_T> FilePosition::fileCodesBimap_ = {};
bimap<std::string, SIZE_T>& FilePosition::fileCodesBimap() {
  return fileCodesBimap_;
}

FilePosition FilePosition::from_string(const std::string& s) {
  std::string fileName;
  SIZE_T row, col;
  SIZE_T i = 0, j = 0;
  while (i < s.size() && s[i] != ':') ++i;
  if (i == s.size())
    throw std::invalid_argument(
        strfast() << "FilePosition: usage: fileName:row:col, given: " << s);

  fileName = s.substr(j, i - j);
  ++i, j = i;

  while (i < s.size() && s[i] != ':') ++i;
  if (i == s.size())
    throw std::invalid_argument(
        strfast() << "FilePosition: usage: fileName:row:col, given: " << s);

  row = atoll(s.substr(j, i - j).c_str());
  ++i, j = i;
  col = atoll(s.substr(j).c_str());

  return FilePosition{std::make_shared<std::string>(fileName), row, col};
}
FileRange FileRange::fromString(const std::string& s) {
  char temp;
  SIZE_T code;
  std::pair<row_t, column_t> begin, end;
  std::istringstream ss(s);
  ss >> code >> temp >> begin.first >> temp >> begin.second >> temp >>
      end.first >> temp >> end.second;

  return FileRange{.fileCode = code, .begin = begin, .end = end};
}
