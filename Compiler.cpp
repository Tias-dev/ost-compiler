#include "Compiler.hpp"
#include "AST.hpp"
#include "BreakPointer.hpp"
#include "CharStream.hpp"
#include "FilePosition.hpp"
#include "Tokenizer.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <bits/getopt_core.h>
#include <cctype>
#include <chrono>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <stdexcept>
const char * helpMessage = 
    "ost language compiler\n"
    "usage: ost <program>.ost [OPTIONS]\n"
		"Description:\n"
		"Compiles <program>.ost program to Machine Turing's commands sequence\n"
		"Name of output file is the name of first MT described in <program>.ost file\n"
		"\n"
		"Awailable options are shown below:\n"
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
		"-h, --help : print this help message and quit\n"
		"\n"
;

void parseCommandArgs(int argc, char *argw[]) {
  size_t nopts = 7;
  option *options = new option[nopts]{
      {.name = "libdir", .has_arg = 1, .flag = NULL, .val = 'l'},
      {.name = "outputdir", .has_arg = 1, .flag = NULL, .val = 'o'},
      {.name = "enable-breakpoints", .has_arg = 0, .flag = NULL, .val = 'g'},
      {.name = "print-debug-info", .has_arg = 0, .flag = NULL, .val = 'd'},
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
using duration_t = std::chrono::milliseconds;
const char * durationSuffix = "ms";

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
    globals::breakpointer = new FileBreakpointer{};

  std::fstream file(fileName);
  if (!file.is_open())
    throw std::invalid_argument(strfast() << "Can't open file: " << fileName);

	auto start_ts = std::chrono::system_clock::now();
  CharStream stream(file);

  auto tokenizer = token::Tokenizer{};
  FileRoller roller{std::make_shared<std::string>(fileName)};
  auto tokens = tokenizer.parse(stream, roller);
  file.close();

  {
		auto start = std::chrono::system_clock::now();
		logger::debug out;
    out << "Tokenizer output:" << std::endl;
    for (auto &token : tokens)
      out << token.toString() << std::endl;
    out << "-----------------------------" << std::endl << std::endl;
		auto end = std::chrono::system_clock::now();
		out << "Tokenizing time:" << std::chrono::duration_cast<duration_t>(end - start).count() << durationSuffix;
  }

  ast::Tree astTree{tokens, fileName};
  {
		auto start = std::chrono::system_clock::now();
		logger::debug out;
    out << "Created AST tree:" << std::endl;
    astTree.print(out);
    out << "-----------------------------" << std::endl << std::endl;
    out << "Names table:" << std::endl;
    ast::MT::printNamesTable(out);
		auto end = std::chrono::system_clock::now();
		out << "AST creating time:" << std::chrono::duration_cast<duration_t>(end - start).count() << durationSuffix;
  }
	std::string foutName = strfast() << globals::outDir << astTree.getTreeName() << ".tu4";

	compiler::commands_type  commands;
	{
		auto start = std::chrono::system_clock::now();
		commands = astTree.to4();
		auto end = std::chrono::system_clock::now();
		logger::debug() << "Compiling from AST to mt4 form time: " << std::chrono::duration_cast<duration_t>(end - start).count() << durationSuffix;
	}
	{
		auto start = std::chrono::system_clock::now();
		compiler::serializer::serialize(commands, FilePosition::fileCodesBimap(), foutName, globals::useBinaryFormat);
		auto end = std::chrono::system_clock::now();
		logger::debug() << "Serializing compiled program time: " << std::chrono::duration_cast<duration_t>(end - start).count() << durationSuffix;
	}
	auto end_ts = std::chrono::system_clock::now();
	logger::debug() << "Total compiling time: " << std::chrono::duration_cast<duration_t>(end_ts - start_ts).count() << durationSuffix;
	logger::info() << "Commands written to file: " << foutName;

  return 0;
}
