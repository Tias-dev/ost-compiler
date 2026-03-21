#include "Line.hpp"
#include "Tu4Runner.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <getopt.h>
#include <cstring>


void usage() {
	std::cout << "Usage: tu4run <program name>.tu4 [OPTIONS]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "-d, --print-debug-info : see ost -h" << std::endl;
	std::cout << "-b, --use-binary-format : see ost -h" << std::endl;
	std::cout << "-h, --help : write this message" << std::endl;
}

void parseCommandArgs(int argc, char *argw[]) {
  SIZE_T nopts = 4;
  option *options = new option[nopts]{
      {.name = "print-debug-info", .has_arg = 0, .flag = NULL, .val = 'd'},
      {.name = "use-binary-format", .has_arg = 0, .flag = NULL, .val = 'b'},
      {.name = "help", .has_arg = 0, .flag = NULL, .val = 'h'}};
  memset(&options[nopts - 1], 0, sizeof(option));

  int arg, longindex;
  while ((arg = getopt_long(argc, argw, "dbh", options, &longindex)) != -1) {
    switch (arg) {
    case '?':
      logger::warning()
          << "Unrecognized option: " << optarg << std::endl;
      break;
    case 'd':
      globals::printDebugInfo = true;
      logger::info() << "Printing debug info enabled";
			break;
    case 'b':
      globals::useBinaryFormat = true;
      logger::info() << "Binary format to loading enabled";
			break;
		case 'h':
			usage();
			exit(0);
			break;
    default:
      logger::warning()
          << "Given option: [" << char(arg) << "] can't be processed";
    }
  }
  delete[] options;
}

using duration_t = std::chrono::milliseconds;
const char * durationSuffix = "ms";

int main(int argc, char * argw[]) {
	if(argc < 2) {
		usage();
		return 1;
	}
	std::string fileName = argw[1];
	parseCommandArgs(argc, argw);

	std::string s;
	std::getline(std::cin, s);
	auto start_ts = std::chrono::system_clock::now();

	auto runner = tu4run::initRunner(fileName, s);
	auto end_ts = std::chrono::system_clock::now();
	logger::debug() << "Command loading: " << std::chrono::duration_cast<duration_t>(end_ts - start_ts).count() << durationSuffix;
		
	

	start_ts = std::chrono::system_clock::now();
	SIZE_T steps = runner->loop();
	end_ts = std::chrono::system_clock::now();

	logger::debug() << "Executing program:" << std::chrono::duration_cast<duration_t>(end_ts - start_ts).count() << durationSuffix;


	std::cout << "Steps: " << steps << " Out line:" << std::endl;
	std::cout << runner->line() << std::endl;

	return 0;
}
