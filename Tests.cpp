#include "AST.hpp"
#include "CharStream.hpp"
#include "Line.hpp"
#include "Tokenizer.hpp"
#include "Tu4Runner.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
const char *help =
    "osttest [TEST .ost]\n"
    "osttest -- ost toolkit testings utility\n"
    "Works in given -- expected format\n"
    "\n"
    "TEST format:\n"
    "TEST is the usual .ost file with:\n"
    "1. [line] one line comment at the top of the file -- it is base line "
    "state\n"
    "2. [number: line] one line comment at the bottom of the file -- it is "
    "expected line state, number is cursor pos\n"
    "\n"
    "Output:\n"
    "Returns 0 if test passed i.e. result line state equal expected line state "
    "including cursor position and non 0 otherwise\n"
    "If any exceptions happens, print exception what() and return also non 0\n";

int main(int argc, char *argw[]) {
  if (argc < 2) {
    std::cout << help << std::endl;
    return 1;
  }

  std::string given, expected, temp;
  size_t cursorExpected;

  std::ifstream file(argw[1]);
  if (file.bad())
    throw std::invalid_argument(strfast() << "Can't open file: " << argw[1]);

  std::getline(file, given);
  while (!file.eof()) {
    std::getline(file, temp);
    if (!file.eof())
      expected = temp;
  }

  given = given.substr(1, given.size() - 2);
  expected = expected.substr(1, expected.size() - 2);

  std::stringstream ss(expected);
  ss >> cursorExpected >> temp;
  std::getline(ss, expected);

	tu4run::Line<char> givenLine{given}, expectedLine{expected};

	file.close();
	file.open(argw[1]);

	CharStream stream{file};
	
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
	
	tu4run::Tu4Runner<size_t, char> runner{givenLine, commands};
	runner.loop();
	if(givenLine != expectedLine) {
		std::cout << "Error:" << std::endl;
		std::cout << "Expected: " << expectedLine.cursor() << ": [" << expectedLine.line() << "]" << std::endl;
		std::cout << "Given: " << givenLine.cursor() << ": [" << givenLine.line() << "]" << std::endl;
		return 1;
	}
	std::cout << "Passed" << std::endl;
  return 0;
}
