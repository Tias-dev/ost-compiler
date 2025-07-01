#include "BreakPointer.hpp"
#include "FilePosition.hpp"
#include "utils.hpp"
#include <cstdlib>

void FileBreakpointer::onEnter(const FilePosition &beginPos,
                               const FilePosition &endPos) {
  states_.push({.begin = beginPos, .end = endPos});
	lastState_ = states_.top();
}
void FileBreakpointer::onExit() {
  if (states_.empty())
		return;
	lastState_ = states_.top();
  states_.pop();
}

std::string FileBreakpointer::getCurrentPosition() {
  if (states_.empty())
    return State::dump(lastState_);
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
