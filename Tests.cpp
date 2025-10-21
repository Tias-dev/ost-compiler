#include "Compiler.hpp"
#include "Line.hpp"
#include "Tu4Runner.hpp"
#include "exception.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <getopt.h>

const char *help =
    "osttest [COMPILED PROGRAM .tu4] [TESTS plaintext] [OPTIONS]\n"
    "osttest -- ost toolkit testings utility\n"
    "Works in given -- expected format\n"
    "\n"
    "PROGRAM format:\n"
    "PROGRAM is the compiled .tu4 program file\n"
		"TEST is the plaintext file with pairs of 2 or 3 lines:\n"
    "1. first line is given line state\n"
    "2. second line is expected line state\n"
		"3. OPTIONAL: cursor position(counting from 0) at the end of program(if not specified it will be after last non space cell)\n"
		"4. Tests delimeter: line of any number of '-' symbols\n"
    "\n"
    "Output:\n"
    "Returns 0 if all tests are passed i.e. result line state equal expected line state for all tests \n"
    "including cursor position and non 0 otherwise\n"
    "If any exceptions happens, print exception what() and return also non 0\n"
		"OPTIONS:\n"
		"\n"
		"-b, --use-binary-format : use binary format for loading program\n";

void parseCommandArgs(int argc, char *argw[]) {
  size_t nopts = 3;
  option *options = new option[nopts]{
      {.name = "use-binary-format", .has_arg = 0, .flag = NULL, .val = 'b'},
      {.name = "help", .has_arg = 0, .flag = NULL, .val = 'h'}};
  memset(&options[nopts - 1], 0, sizeof(option));

  int arg, longindex;
  while ((arg = getopt_long(argc, argw, "l:o:gdbh", options, &longindex)) != -1) {
    switch (arg) {
    case '?':
      logger::warning()
          << "Unrecognized option: " << optarg << std::endl;
      break;
    case 'b':
      globals::useBinaryFormat = true;
      logger::info() << "Binary format to saving enabled";
			break;
		case 'h':
			std::cout << help << std::endl;
			exit(0);
			break;
    default:
      logger::warning()
          << "Given option: [" << char(arg) << "] can't be processed";
    }
  }
  delete[] options;
}

int main(int argc, char *argw[]) {
  if (argc < 3) {
    std::cout << help << std::endl;
    return 1;
  }

  size_t cursorExpected;

	std::string fileName = argw[1], testFileName = argw[2];
	parseCommandArgs(argc, argw);
	auto commands = compiler::serializer::deserialize(fileName, globals::useBinaryFormat);

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
		
			
		tu4run::Line<char> expectedLine{expected};
		if(cursor != -1) {
			while(cursor > expectedLine.cursor()) 
				expectedLine.shiftRight();
			while(cursor < expectedLine.cursor())
				expectedLine.shiftLeft();
		}
			
		tu4run::Tu4Runner<size_t, char> runner{tu4run::Line<char>(given), commands};
		runner.loop();
		if(runner.line() != expectedLine) {
			std::cout << i_test<< ") Error:" << std::endl;
			std::cout << "Expected: " << expectedLine.cursor() << ": [" << expectedLine.line() << "]" << std::endl;
			std::cout << "Given: " << runner.line().cursor() << ": [" << runner.line().line() << "]" << std::endl;
			wasErrors = true;
		} else {
			std::cout << i_test << ") Passed" << std::endl;
		}
		given = "", expected = "", delimeter = "", ++i_test;
	}

  return wasErrors ? 1 : 0;
}
