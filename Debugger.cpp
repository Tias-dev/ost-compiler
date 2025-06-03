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
    "input: 'b -q <state number>' to set breakpoint on 'q' state\n"
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
  if (command.isTerm())
    return;

  auto comment = command.comment();
  if (comment.empty()) {
    std::cout << "No debug information in line: " << command << std::endl;
    return;
  }
  auto state = FileBreakpointer::State::load(comment);
  const std::string &fileName = state.begin.fileName();
  const FilePosition &begin = state.begin, &end = state.end;
  std::string line;

  if (nLine > begin.row()) {
    if (!file.is_open())
      file.open(fileName);
    else {
      file.clear();
      file.seekg(0);
    }
    nLine = 1;
  }

  while (!file.eof() && nLine <= begin.row()) {
    std::getline(file, line);
    ++nLine;
  }

  for (auto &c : line)
    if (c == '\t')
      c = ' ';

  std::cout << nLine << " :" << line << std::endl;
  std::cout << nLine << " :";
  for (size_t i = 0; i+1 < begin.column(); ++i)
    std::cout << " ";
  size_t endIndex = (begin.row() == end.row() ? end.column() : line.size());
  for (size_t i = begin.column(); i < endIndex; ++i)
    std::cout << "^";
  std::cout << std::endl;
}

int main(int argc, char *argw[]) {
  auto commands = initCommands();

  std::cout << preview;
  std::string line;
  tu4run::Tu4Runner<size_t, char> *runner = nullptr;
  std::string fileName;
  size_t n = 1;
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
      runner = tu4run::initRunner(fileName, line);
      break;
    case Command::STEP:
      if (words.size() > 2 && *(++word) == "-n") {
        n = atoll(word->c_str());
      }
      for (size_t i = 0; i < n && !runner->step(); ++i) {
      }

      n = 1;
      break;
    case Command::GO:
      runner->loop();
      std::cout << runner->line() << std::endl;
      break;
    default:
      std::cout << "Warning: command not supported: " << *word << std::endl;
    }
    if (runner && !runner->terminated()) {
      std::cout << runner->line();
      printState(*runner);
    }
  }

  return 0;
}
