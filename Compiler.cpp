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
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <stdexcept>
const char * helpMessage = "";

void parseCommandArgs(int argc, char *argw[]) {
  size_t nopts = 6;
  option *options = new option[nopts]{
      {.name = "libdir", .has_arg = 1, .flag = NULL, .val = 'l'},
      {.name = "outputdir", .has_arg = 1, .flag = NULL, .val = 'o'},
      {.name = "enable-breakpoints", .has_arg = 0, .flag = NULL, .val = 'g'},
      {.name = "print-debug-info", .has_arg = 0, .flag = NULL, .val = 'd'},
      {.name = "help", .has_arg = 0, .flag = NULL, .val = 'h'}};
  memset(&options[nopts - 1], 0, sizeof(option));

  int arg, longindex;
  while ((arg = getopt_long(argc, argw, "l:o:gdh", options, &longindex)) != -1) {
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
      globals::libDir = optarg;
      logger::info() << "Used library directory: " << optarg;
      break;
    case 'g':
      globals::enableBreakpoints = true;
      logger::info() << "Breakpoints enabled";
      break;
    case 'd':
      globals::printDebugInfo = true;
      logger::info() << "Printing debug info enabled";
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
    globals::breakpointer = new FileBreakpointer{fileName};

  std::fstream file(fileName);
  if (!file.is_open())
    throw std::invalid_argument(strfast() << "Can't open file: " << fileName);

  CharStream stream(file);

  auto tokenizer = token::Tokenizer{};
  FileRoller roller{std::make_shared<std::string>(fileName)};
  auto tokens = tokenizer.parse(stream, roller);
  file.close();

  {
		logger::debug out;
    out << "Tokenizer output:" << std::endl;
    for (auto &token : tokens)
      out << token.toString() << std::endl;
    out << "-----------------------------" << std::endl << std::endl;
  }

  ast::Tree astTree{tokens, fileName};
  {
		logger::debug out;
    out << "Created AST tree:" << std::endl;
    astTree.print(out);
    out << "-----------------------------" << std::endl << std::endl;
    out << "Names table:" << std::endl;
    ast::MT::printNamesTable(out);
  }
	std::string foutName = strfast() << globals::outDir << astTree.getTreeName() << ".tu4";

  std::ofstream fout(foutName);
	if(globals::enableBreakpoints) 
		fout << globals::debugFirstLine << "// Please not modify comment in program below\n";
		
	compiler::commands_type commands = astTree.to4();
  for (auto &command : commands)
    fout << command << '\n';

	logger::info() << "Command written to file: " << foutName;

  return 0;
}
