#include "AST.hpp"
#include "CharStream.hpp"
#include "Line.hpp"
#include "Tokenizer.hpp"
#include "Tu4Runner.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
const char *help =
    "osttest [PROGRAM .ost] [TESTS plaintext]\n"
    "osttest -- ost toolkit testings utility\n"
    "Works in given -- expected format\n"
    "\n"
    "PROGRAM format:\n"
    "PROGRAM is the usual .ost file\n"
		"TEST is the plaintext file with pairs of 2/(3) lines:"
    "1. first line is given line state"
    "2. second line is expected line state\n"
		"3. OPTIONAL: cursor position at the end of program(if not specified it will be after last non space cell)"
		"4. Tests delimeter: line of any number of '-' symbols"
    "\n"
    "Output:\n"
    "Returns 0 if all tests are passed i.e. result line state equal expected line state for all tests "
    "including cursor position and non 0 otherwise\n"
    "If any exceptions happens, print exception what() and return also non 0\n";

int main(int argc, char *argw[]) {
  if (argc < 2) {
    std::cout << help << std::endl;
    return 1;
  }

  size_t cursorExpected;

  std::ifstream programFile(argw[1]);
  if (!programFile.good())
    throw std::invalid_argument(strfast() << "Can't open file: " << argw[1]);
	CharStream stream{programFile};
	token::Tokenizer tokenizer{};
	token::tokens_list tokens;
	try {
		tokens = tokenizer.parse(stream);
	} catch (error::PositionErrorBase & e) {
    std::cout << "Error: " << e.what() << std::endl;
		fileRollAround(argw[1], e.position(), 60);
		return 1;
	}
	ast::Tree tree{tokens, argw[1]};
	auto commands = tree.to4();

	std::ifstream testsFile(argw[2]);
	size_t i_test = 1;
	std::string given = "", expected, delimeter;

	bool wasErrors = false;
	while(!testsFile.eof()) {
		long long cursor = -1;

		if(given == "") {
			std::getline(testsFile, given);
			continue;
		} 

		std::getline(testsFile, expected);
		std::getline(testsFile, delimeter);
		delimeter = strip(delimeter);
		if(delimeter.size() > 0 && delimeter[0] >= '0' && delimeter[0] <= '9') {
			cursor = atol(delimeter.c_str());
			std::getline(testsFile, delimeter);
		}

		for(auto& c : delimeter) 
			if(c != '-') 
				throw std::invalid_argument(strfast() << "Tests delimeter awaits to consist of '-' only, [" << c << "] given");
		
			
		tu4run::Line<char> givenLine{given}, expectedLine{expected};
		if(cursor != -1) {
			while(cursor > expectedLine.cursor()) 
				expectedLine.shiftRight();
			while(cursor < expectedLine.cursor())
				expectedLine.shiftLeft();
		}
			
		tu4run::Tu4Runner<size_t, char> runner{givenLine, commands};
		runner.loop();
		if(givenLine != expectedLine) {
			std::cout << i_test<< ") Error:" << std::endl;
			std::cout << "Expected: " << expectedLine.cursor() << ": [" << expectedLine.line() << "]" << std::endl;
			std::cout << "Given: " << givenLine.cursor() << ": [" << givenLine.line() << "]" << std::endl;
			wasErrors = true;
		} else {
			std::cout << i_test << ") Passed" << std::endl;
		}
		given = "", expected = "", delimeter = "", ++i_test;
	}

  return wasErrors ? 1 : 0;
}
