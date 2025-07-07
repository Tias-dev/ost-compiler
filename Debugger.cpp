#include "BreakPointer.hpp"
#include "FilePosition.hpp"
#include "Line.hpp"
#include "Tu4Command.hpp"
#include "Tu4Runner.hpp"
#include "trie.hpp"
#include "utils.hpp"
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
    "input: 'b -q <state number>' to set breakpoint on <state number>(from 0) "
    "state\n"
    "input: 'b -l <line number>' to set breakpoint on <line number>(from 1) "
    "line\n"
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

struct FileData {
  std::ifstream file;
  size_t nColumn = 0;
  size_t nLine = 1e18;
};
void printState(const tu4run::Tu4Runner<size_t, char> &runner) {
  static std::map<std::string, FileData> filesData;

  auto command = runner.nextCommand();
  auto comment = command.comment();
  if (comment.empty()) {
    logger::debug() << "No debug information in line: "
                    << (strfast() << command).bump() << std::endl;
    return;
  }
  auto state = FileBreakpointer::State::load(comment);
  const std::string &fileName = state.begin.fileName();
  const FilePosition &begin = state.begin, &end = state.end;
  std::string line;

  const FilePosition *position = &begin;
  if (runner.terminated())
    position = &end;

  if (!filesData.contains(fileName))
    filesData[fileName] = FileData{};
  FileData &fdata = filesData[fileName];

  if (fdata.nLine > position->row()) {
    if (!fdata.file.is_open())
      fdata.file.open(fileName);
    else {
      fdata.file.clear();
      fdata.file.seekg(0);
    }
    fdata.nLine = 1;
  }

  while (!fdata.file.eof() && fdata.nLine <= position->row()) {
    std::getline(fdata.file, line);
    ++fdata.nLine;
  }

  for (auto &c : line)
    if (c == '\t')
      c = ' ';

  std::cout << fdata.nLine - 1 << " :" << line << std::endl;
  std::cout << fdata.nLine - 1<< " :";
  if (!runner.terminated())
    for (size_t i = 0; i + 1 < position->column(); ++i)
      std::cout << " ";
  size_t endIndex = (begin.row() == end.row() ? end.column() : line.size());
  for (size_t i = position->column(); i < endIndex; ++i)
    std::cout << "^";
  std::cout << std::endl;
  std::cout << "command: " << command << std::endl;
}

class Manager {
  impl::Trie<Command> commands_;
  std::unique_ptr<tu4run::Tu4Runner<size_t, char>> runner_ = nullptr;
  tu4run::Tu4RunnerBreakpoints breakpoints_;
  std::vector<std::string> usedFileNames_;

  void run(const std::string &fileName) {
    std::string line;
    std::cout << "Input line: ";
    std::getline(std::cin, line);
    auto runnerInitData = tu4run::initRunnerWithBreakpoints(fileName, line);
    runner_ = std::move(runnerInitData.runner);
    breakpoints_ = runnerInitData.breakpoints;
    usedFileNames_ = runnerInitData.fileNames;
  }

  void step(size_t n = 1) {
    while (n-- >= 1 && !runner_->terminated())
      runner_->step();
  }

  void go() {
    runner_->loop();
    if (runner_->terminated())
      std::cout << "End" << std::endl;
    else
      std::cout << "Stop at breakpoint" << std::endl;
  }

  void addBreakpointState(size_t q) { breakpoints_.stateBreakpoints->add(q); }

  void addBreakpointLine(size_t nLine) {
    std::cout << "Choose file name in which set line breakpoint:\n";
    for (size_t i = 0; i < usedFileNames_.size(); ++i)
      std::cout << i << ") " << usedFileNames_[i] << '\n';
    size_t findex;
    std::cout << "Enter file number: ";
    std::cin >> findex;

    if (!breakpoints_.lineBreakpoints->contains(usedFileNames_.at(findex)))
      (*breakpoints_.lineBreakpoints)[usedFileNames_.at(findex)] = {};

    if (nLine > 0)
      (*breakpoints_.lineBreakpoints)[usedFileNames_[findex]].add(nLine);
  }

  void processLine_impl(const std::list<std::string> &words) {
    if (words.size() == 0) {
      if (!runner_)
        return;
      if (!runner_->terminated())
        runner_->step();
			return;
    }

    std::string line;
    auto word = std::begin(words);
    Command command = commands_.find(*word).value_or(Command::NONE);

    switch (command) {
    case Command::NONE:
      std::cout << "Error: undefined command: " << *word << std::endl;
      break;
    case Command::RUN:
      if (words.size() < 2) {
        logger::warning() << "Error: file name to run not given";
        return;
      }
      run(*(++word));
      break;
    case Command::STEP:
      if (words.size() > 2 && *(++word) == "-n")
        step(atoll(std::next(word)->c_str()));
      else
        step(1);
      break;
    case Command::GO:
      go();
      break;
    case Command::B:
      ++word;
      if (word == std::end(words)) {
        std::cout << "Add -q or -l flag to set type of breakpoint" << std::endl;
        break;
      }
      if (*word == "-q") {
        addBreakpointState(atoll(std::next(word)->c_str()));
      } else if (*word == "-l") {
        size_t nLine = atoll(std::next(word)->c_str());
        addBreakpointLine(nLine);
      } else {
        std::cout << "Warning: unrecognized breakpoints type. Awaited -q or "
                     "-l. Given: "
                  << *word << std::endl;
      }
      break;
    default:
      std::cout << "Warning: command not supported: " << *word << std::endl;
    }
  }

public:
  Manager() : commands_(initCommands()) {}

  void processLine(const std::string &commandLine) {
    auto words = split(commandLine);
    processLine_impl(words);

    if (runner_) {
      std::cout << runner_->line();
      printState(*runner_);
    }
  }
};

int main(int argc, char *argw[]) {
  Manager manager;
  std::cout << preview;
  std::string line;

  while (!std::cin.eof()) {
    std::getline(std::cin, line);
		if(line == "help") {
			std::cout << preview;
			continue;
		}
			
    manager.processLine(line);
  }

  return 0;
}
