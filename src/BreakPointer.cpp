#include "BreakPointer.hpp"

#include <sstream>

#include "FilePosition.hpp"
#include "utils.hpp"

void FileBreakpointer::onEnter(const FileRange& range) {
  states_.push({.range = range});
  lastState_ = states_.top();
}
void FileBreakpointer::onExit() {
  if (states_.empty()) return;
  lastState_ = states_.top();
  states_.pop();
}

FileRange FileBreakpointer::getCurrentPosition() {
  if (states_.empty()) return lastState_.range;
  return states_.top().range;
}

std::string FileBreakpointer::State::dump(const State& state) {
  return strfast() << state.range;
}

FileBreakpointer::State FileBreakpointer::State::load(const std::string& s) {
  char temp;
  SIZE_T code;
  std::pair<row_t, column_t> begin, end;
  std::istringstream ss(s);
  ss >> code >> temp >> begin.first >> temp >> begin.second >> temp >>
      end.first >> temp >> end.second;

  return State{FileRange{.fileCode = code, .begin = begin, .end = end}};
}
