#include "BreakPointer.hpp"
#include "FilePosition.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <memory>
#include <sstream>
#include <stdexcept>

void FileBreakpointer::onEnter(size_t beginPos,
                               size_t endPos) {
	states_.push({beginPos, endPos});
}
void FileBreakpointer::onExit() {
	if(states_.empty()) 
		throw std::logic_error("FileBreakpointer: exit before enter");
	states_.pop();
}

std::string FileBreakpointer::getCurrentPosition() {
	if(states_.empty()) 
		throw std::logic_error("No states found");
	return State::dump(State{fileNamePtr_, states_.top().second});
}

std::string FileBreakpointer::State::dump(const State & state) {
	return state.to_string();
}

FileBreakpointer::State FileBreakpointer::State::load(const std::string &s) {
	std::istringstream iss(s);
	std::string fileName;
	
	auto throwError = [&s]() {
		throw std::invalid_argument(strfast() << "Invalid state format, expected: [fileName, beginPos, endPos], but given: [" << s<< "]\n");
	};

	size_t row, column;
	size_t j = 0, i = 1;
	while(i < s.size() && s[i] != ':') ++i;
	if(i == s.size()) 
		throwError();

	fileName = s.substr(j, i - j);
	++i;
	j = i;

	while(i < s.size() && s[i] != ':') ++i;
	if(i == s.size()) 
		throwError();

	row = atoll(s.substr(j, i - j).c_str());
	column = atoll(s.substr(i + 1).c_str());

	return State{std::make_shared<std::string>(fileName), row, column, 0};
}
