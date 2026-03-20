#include "Compiler.hpp"
#include "BreakPointer.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <bits/getopt_core.h>
#include <cctype>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <stdexcept>
#include <string>
const char * helpMessage = 
    "ost language compiler\n"
    "usage: ost <program>.ost [OPTIONS]\n"
		"Description:\n"
		"Compiles <program>.ost program to Machine Turing's commands sequence\n"
		"Name of output file is the name of first MT described in <program>.ost file\n"
		"\n"
		"Awailable options are shown below:\n"
		"-h, --help : print this help message and quit\n"
		"-v, --verbose : detailed output if --print-debug-info enabled\n"
		"-l, --libdir : directory where precompiled libraries(.tu4, not source .ost files!) are stored\n"
		"\t(requires argument, default: ./)\n"
		"-o, --outputdir : directory where compiled <program>.ost program will be stored\n"
		"\t(requires argument, default: ./)\n"
		"-g, --enable-breakpoints : enabling breakpoints setting option. Program compiled with this flag can be debugged with ostdb utility\n"
		"\t(default: disabled)\n"
		"-d, --print-debug-info : enabling debug info\n"
		"\t(default: disabled)\n"
		"-b, --use-binary-format : save compiled sequence of commands in binary format. Significant speed up to loading and saving\n"
		"\t(default: disabled)\n"
		"\n"
;

void parseCommandArgs(int argc, char *argw[]) {
  size_t nopts = 8;
  option *options = new option[nopts]{
      {.name = "libdir", .has_arg = 1, .flag = NULL, .val = 'l'},
      {.name = "outputdir", .has_arg = 1, .flag = NULL, .val = 'o'},
      {.name = "enable-breakpoints", .has_arg = 0, .flag = NULL, .val = 'g'},
      {.name = "print-debug-info", .has_arg = 0, .flag = NULL, .val = 'd'},
      {.name = "use-binary-format", .has_arg = 0, .flag = NULL, .val = 'b'},
      {.name = "verbose", .has_arg = 0, .flag = NULL, .val = 'v'},
      {.name = "help", .has_arg = 0, .flag = NULL, .val = 'h'}};
  memset(&options[nopts - 1], 0, sizeof(option));

  int arg, longindex;
  while ((arg = getopt_long(argc, argw, "l:o:gdbhv", options, &longindex)) != -1) {
    switch (arg) {
    case '?':
      logger::warning()
          << "Unrecognized option: " << optarg << std::endl;
      break;
    case 'v':
      globals::verboseOutput = true;
      logger::info() << "Verbose output enabled";
      break;
    case 'l':
      globals::libDir = optarg;
      logger::info() << "Used library directory: " << optarg;
      break;
    case 'o':
      globals::outDir = optarg;
      logger::info() << "Used output directory: " << optarg;
      break;
    case 'g':
      globals::enableBreakpoints = true;
      logger::info() << "Breakpoints enabled";
      break;
    case 'd':
      globals::printDebugInfo = true;
      logger::info() << "Printing debug info enabled";
			break;
    case 'b':
      globals::useBinaryFormat = true;
      logger::info() << "Binary format to saving enabled";
			break;
		case 'h':
			std::cout << helpMessage << std::endl;
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
  if (argc < 2)
    throw std::invalid_argument("Usage: ost <program>.ost [OPTIONS]");
	std::string fileName = argw[1];
	parseCommandArgs(argc, argw);

  if (globals::outDir[globals::outDir.size() - 1] != '/')
    globals::outDir.push_back('/');

  if (globals::libDir[globals::libDir.size() - 1] != '/')
    globals::libDir.push_back('/');

  if (globals::enableBreakpoints)
    globals::breakpointer = std::make_shared<FileBreakpointer>();
	compileAndSaveProgram(fileName, globals::libDir, globals::outDir, globals::useBinaryFormat, globals::enableBreakpoints);
  return 0;
}
