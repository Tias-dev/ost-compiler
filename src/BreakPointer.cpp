#include "BreakPointer.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <vector>

void FileBreakpointer::onEnter(size_t beginPos,
                               size_t endPos) {
	states_.push({beginPos, endPos});
}
void FileBreakpointer::onExit() {
	if(states_.empty()) 
		throw std::logic_error("FileBreakpointer: exit before enter");
}
std::string FileBreakpointer::getCurrentPosition() {
	if(states_.empty()) 
		throw std::logic_error("No states found");
	return State::dump(State{fileName_, states_.top().first, states_.top().second});
}

std::string FileBreakpointer::State::dump(const State & state) {
	return strfast() << state.fileName << "," << state.begin << "," << state.end;
}

FileBreakpointer::State FileBreakpointer::State::load(const std::string &s) {
	std::istringstream iss(s);
	std::string fileName;
	
	auto throwError = [&s]() {
		throw std::invalid_argument(strfast() << "Invalid state format, expected: [fileName, beginPos, endPos], but given: [" << s<< "]\n");
	};

	size_t begin, end;
	size_t j = 0, i = 1;
	while(i < s.size() && s[i] != ',') ++i;
	if(i == s.size()) 
		throwError();

	fileName = s.substr(j, i - j - 1);
	j = i;

	while(i < s.size() && s[i] != ',') ++i;
	if(i == s.size()) 
		throwError();

	begin = atoll(s.substr(j, i - j - 1).c_str());
	end = atoll(s.substr(i + 1).c_str());

	return State{fileName, begin, end};
}
