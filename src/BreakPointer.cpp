#include "BreakPointer.hpp"
#include "FilePosition.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <memory>
#include <sstream>
#include <stdexcept>

void FileBreakpointer::onEnter(const FilePosition &beginPos,
                               const FilePosition &endPos) {
  states_.push({.begin = beginPos, .end = endPos});
}
void FileBreakpointer::onExit() {
  if (states_.empty())
    throw std::logic_error("FileBreakpointer: exit before enter");
  states_.pop();
}

std::string FileBreakpointer::getCurrentPosition() {
  if (states_.empty())
    throw std::logic_error("No states found");
  return State::dump(states_.top());
}

std::string FileBreakpointer::State::dump(const State &state) {
  return strfast() << state.begin.to_string() << ";" << state.end.to_string();
}

FileBreakpointer::State FileBreakpointer::State::load(const std::string &s) {
  size_t i = 0;
  while (s[i] != ';')
    ++i;

  return State{.begin = FilePosition::from_string(s.substr(0, i)),
               .end = FilePosition::from_string(s.substr(i + 1))};
}
