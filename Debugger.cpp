#include "BreakPointer.hpp"
#include "FilePosition.hpp"
#include "Line.hpp"
#include "Tu4Command.hpp"
#include "Tu4Runner.hpp"
#include "trie.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

const char *preview =
    "ost programm debugger\n"
    "input: 'run <program name .tu4>' to start debug(program must be compiled "
    "with -g flag)\n"
    "input: 'step' or empty line to do step and show file state\n"
    "input: 'step -n [number]' to do 'n' steps and show file state\n"
    "input: 'go' to start stepping loop until program's end or breakpoint "
    "occured\n"
    "input: 'b -q <state number>' to set breakpoint on <state number>(from 0) state\n"
    "input: 'b -l <line number>' to set breakpoint on <line number>(from 1) line\n"
    "(Experimental)input: 'b --name <string without spaces>' to set breakpoint "
    "on 'name' prefix of command field of view\n"
    "input: 'help' show this message\n";

enum class Command { NONE, RUN, STEP, GO, B, HELP };

impl::Trie<Command> initCommands() {
  impl::Trie<Command> trie;
  trie.add("run", Command::RUN);
  trie.add("step", Command::STEP);
  trie.add("go", Command::GO);
  trie.add("b", Command::B);
  trie.add("help", Command::HELP);

  return trie;
}

void printState(const tu4run::Tu4Runner<size_t, char> &runner) {
  static std::ifstream file;
  static size_t nColumn = 0;
  static size_t nLine = 1e18;

  auto command = runner.nextCommand();
  auto comment = command.comment();
  if (comment.empty()) {
    std::cout << "No debug information in line: " << command << std::endl;
    return;
  }
  auto state = FileBreakpointer::State::load(comment);
  const std::string &fileName = state.begin.fileName();
  const FilePosition &begin = state.begin, &end = state.end;
  std::string line;

	const FilePosition * position = &begin;
	if(runner.terminated()) 
		position = &end;

	if (nLine > position->row()) {
    if (!file.is_open())
      file.open(fileName);
    else {
      file.clear();
      file.seekg(0);
    }
    nLine = 1;
  }

  while (!file.eof() && nLine <= position->row()) {
    std::getline(file, line);
    ++nLine;
  }

  for (auto &c : line)
    if (c == '\t')
      c = ' ';

  std::cout << nLine << " :" << line << std::endl;
  std::cout << nLine << " :";
	if(!runner.terminated()) 
		for (size_t i = 0; i+1 < position->column(); ++i)
			std::cout << " ";
	size_t endIndex = (begin.row() == end.row() ? end.column() : line.size());
  for (size_t i = position->column(); i < endIndex; ++i)
    std::cout << "^";
  std::cout << std::endl;
	std::cout << command << std::endl;
	std::cout << "current q: " << runner.q() << std::endl;
}

int main(int argc, char *argw[]) {
  auto commands = initCommands();

  std::cout << preview;
  std::string line;
	std::unique_ptr<tu4run::Tu4Runner<size_t, char>> runner = nullptr;
	tu4run::Tu4RunnerBreakpoints breakpoints;
	std::tuple<std::unique_ptr<tu4run::Tu4Runner<size_t, char>>, tu4run::Tu4RunnerBreakpoints> runnerInitData;

  std::string fileName;
  size_t n = 1;

	std::set<size_t> stateBreakpoints, linesBreakpoints;
  while (!std::cin.eof()) {
    std::getline(std::cin, line);
    auto words = split(line);

    if (words.size() == 0) {
			if(runner && !runner->terminated()) 
				runner->step();
			if (!runner->terminated()) {
				std::cout << runner->line();
				printState(*runner);
			}
				
      continue;
    }

    auto word = std::begin(words);
    Command command = commands.find(*word).value_or(Command::NONE);
    switch (command) {
    case Command::NONE:
      std::cout << "Error: undefined command: " << *word << std::endl;
      continue;
      break;
    case Command::RUN:
      if (words.size() < 2) {
        std::cout << "Error: file name to run not given";
        continue;
      }
      fileName = *(++word);
      std::cout << "Input line: ";
      std::getline(std::cin, line);
      runnerInitData = tu4run::initRunnerWithBreakpoints(fileName, line);
			runner = std::move(std::get<0>(runnerInitData));
			breakpoints = std::get<1>(runnerInitData);
      break;
    case Command::STEP:
      if (words.size() > 2 && *(++word) == "-n") {
        n = atoll(std::next(word)->c_str());
      }
      for (size_t i = 0; i < n && !runner->step(); ++i) {
      }

      n = 1;
      break;
    case Command::GO:
      runner->loop();
			if(runner->terminated()) 
				std::cout << "End" << std::endl;
			else
				std::cout << "Stop at breakpoint" << std::endl;
      break;
		case Command::B:
			++word;
			if(word == std::end(words)) {
				std::cout << "Add -q or -l flag to set type of breakpoint" << std::endl;
				break;
			}
			if(*word == "-q") {
				size_t b = atoll(std::next(word)->c_str());
				breakpoints.stateBreakpoints->add(b);
			} else if(*word == "-l") {
				size_t b = atoll(std::next(word)->c_str());
				if(b > 1) 
					breakpoints.lineBreakpoints->add(b);
			} else {
				std::cout << "Warning: unrecognized breakpoints type. Awaited -q or -l. Given: " << *word << std::endl;
			}
			break;
    default:
      std::cout << "Warning: command not supported: " << *word << std::endl;
    }
    if (runner) {
      std::cout << runner->line();
      printState(*runner);
    }
  }

  return 0;
}
