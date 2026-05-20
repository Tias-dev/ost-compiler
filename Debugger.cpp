#include <getopt.h>
#include <limits.h>
#include <string.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "FilePosition.hpp"
#include "Line.hpp"
#include "Tu4Command.hpp"
#include "Tu4Runner.hpp"
#include "exception.hpp"
#include "globals.hpp"
#include "trie.hpp"
#include "utils.hpp"

const char* preview =
    "ost programm debugger\n"
    "input: 'run <program name .tu4>' to start debug(program must be "
    "compiled "
    "with -g flag)\n"
    "input: 'step' or empty line to do step and show file state\n"
    "input: 'step -n [number]' to do 'n' steps and show file state\n"
    "input: 'go' to start stepping loop until program's end or breakpoint "
    "occured\n"
    "input: 'b -q <state number>' to set breakpoint on <state number>(from "
    "0) "
    "state\n"
    "input: 'b -l <line number>' to set breakpoint on <line number>(from "
    "1) "
    "line\n"
    "input: 'restart' to restart current loadded program with new starting "
    "line\n"
    "input: 'help' show this message\n";

enum class Command { NONE, RUN, STEP, GO, B, HELP, RESTART };

impl::Trie<Command> initCommands() {
  impl::Trie<Command> trie;
  trie.add("run", Command::RUN);
  trie.add("step", Command::STEP);
  trie.add("go", Command::GO);
  trie.add("b", Command::B);
  trie.add("help", Command::HELP);
  trie.add("restart", Command::RESTART);

  return trie;
}

struct FileData {
  std::ifstream file;
  SIZE_T nColumn = 0;
  SIZE_T nLine = std::numeric_limits<SIZE_T>::max();
};
void printState(const tu4run::Tu4Runner<SIZE_T, char>& runner) {
  static std::map<std::string, FileData> filesData;

  auto command = runner.nextCommand();
  auto debugInfo = command.debugBreakpoint();
  if (!debugInfo.has_value()) {
    logger::debug() << "No debug information in line: "
                    << (strfast() << command).bump() << std::endl;
    return;
  }
  const std::string& fileName =
      FilePosition::fileCodesBimap()[debugInfo.value().fileCode];
  std::string line;

  const std::pair<row_t, column_t> begin = debugInfo.value().begin,
                                   end = debugInfo.value().end,
                                   *position = &begin;
  if (runner.terminated()) position = &end;

  if (!filesData.contains(fileName)) filesData[fileName] = FileData{};
  FileData& fdata = filesData[fileName];

  if (fdata.nLine > position->first) {
    if (!fdata.file.is_open())
      fdata.file.open(fileName);
    else {
      fdata.file.clear();
      fdata.file.seekg(0);
    }
    fdata.nLine = 1;
  }

  while (!fdata.file.eof() && fdata.nLine <= position->first) {
    std::getline(fdata.file, line);
    ++fdata.nLine;
  }

  for (auto& c : line)
    if (c == '\t') c = ' ';

  std::cout << fdata.nLine - 1 << " :" << line << std::endl;
  std::cout << fdata.nLine - 1 << " :";
  if (!runner.terminated())
    for (SIZE_T i = 0; i + 1 < position->second; ++i) std::cout << " ";
  SIZE_T endIndex = (begin.first == end.first ? end.second : line.size());
  for (SIZE_T i = position->second; i < endIndex; ++i) std::cout << "^";
  std::cout << std::endl;
  std::cout << "command: " << command << std::endl;
}

class Manager {
  impl::Trie<Command> commands_;
  std::unique_ptr<tu4run::Tu4Runner<SIZE_T, char>> runner_ = nullptr;
  tu4run::Tu4RunnerBreakpoints breakpoints_;
  std::vector<std::string> usedFileNames_;

  void restart() {
    if (!runner_) {
      logger::warning() << "Setup runner by 'run' command before reseting it";
      return;
    }

    std::string line;
    std::cout << "Input line: ";
    std::getline(std::cin, line);
    (*runner_).reset(tu4run::Line<char>{line});
  }

  void run(const std::string& fileName) {
    std::string line;
    std::cout << "Input line: ";
    std::getline(std::cin, line);
    auto runnerInitData = tu4run::initRunnerWithBreakpoints(fileName, line);
    runner_ = std::move(runnerInitData.runner);
    breakpoints_ = runnerInitData.breakpoints;
    usedFileNames_ = runnerInitData.fileNames;
  }

  void step(SIZE_T n = 1) {
    while (n-- >= 1 && !runner_->terminated()) runner_->step();
  }

  void go() {
    try {
      runner_->loop();
      if (runner_->terminated())
        std::cout << "End" << std::endl;
      else
        std::cout << "Stop at breakpoint" << std::endl;
    } catch (error::Tu4RunError<>& e) {
      logger::error() << e.what();
      std::cout << "Forsing restart via error while executing\n";
      restart();
    }
  }

  void addBreakpointState(SIZE_T q) { breakpoints_.stateBreakpoints->add(q); }

  void addBreakpointLine(SIZE_T nLine) {
    std::cout << "Choose file name in which set line breakpoint:\n";
    for (SIZE_T i = 0; i < usedFileNames_.size(); ++i)
      std::cout << i << ") " << usedFileNames_[i] << '\n';
    SIZE_T findex;
    std::cout << "Enter file number: ";
    std::cin >> findex;

    SIZE_T fileCode = FilePosition::fileCodesBimap()[usedFileNames_.at(findex)];

    if (!breakpoints_.lineBreakpoints->contains(fileCode))
      (*breakpoints_.lineBreakpoints)[fileCode] = {};

    if (nLine > 0) (*breakpoints_.lineBreakpoints)[fileCode].add(nLine);
  }

  void processLine_impl(const std::list<std::string>& words) {
    if (words.size() == 0) {
      if (!runner_) return;
      if (!runner_->terminated()) runner_->step();
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
          std::cout << "Add -q or -l flag to set type of breakpoint"
                    << std::endl;
          break;
        }
        if (*word == "-q") {
          addBreakpointState(atoll(std::next(word)->c_str()));
        } else if (*word == "-l") {
          SIZE_T nLine = atoll(std::next(word)->c_str());
          addBreakpointLine(nLine);
        } else {
          std::cout << "Warning: unrecognized breakpoints type. Awaited -q or "
                       "-l. Given: "
                    << *word << std::endl;
        }
        break;
      case Command::RESTART:
        restart();
        break;
      default:
        std::cout << "Warning: command not supported: " << *word << std::endl;
    }
  }

 public:
  Manager() : commands_(initCommands()) {}

  void processLine(const std::string& commandLine) {
    auto words = split(commandLine);
    processLine_impl(words);

    if (runner_) {
      std::cout << runner_->line();
      printState(*runner_);
    }
  }
};

void parseCommandArgs(int argc, char* argw[]) {
  SIZE_T nopts = 3;
  option* options = new option[nopts]{
      {.name = "use-binary-format", .has_arg = 0, .flag = NULL, .val = 'b'},
      {.name = "help", .has_arg = 0, .flag = NULL, .val = 'h'}};
  memset(&options[nopts - 1], 0, sizeof(option));

  int arg, longindex;
  while ((arg = getopt_long(argc, argw, "bh", options, &longindex)) != -1) {
    switch (arg) {
      case '?':
        logger::warning() << "Unrecognized option: " << optarg << std::endl;
        break;
      case 'b':
        globals::useBinaryFormat = true;
        logger::info() << "Binary format to loading enabled";
        break;
      case 'h':
        std::cout << "flags b and h. see ost -h for details" << '\n';
        exit(0);
        break;
      default:
        logger::warning() << "Given option: [" << char(arg)
                          << "] can't be processed";
    }
  }
  delete[] options;
}

int main(int argc, char* argw[]) {
  globals::enableBreakpoints = true;
  parseCommandArgs(argc, argw);
  Manager manager;
  std::cout << preview;
  std::string line;

  while (!std::cin.eof()) {
    std::getline(std::cin, line);
    if (line == "help") {
      std::cout << preview;
      continue;
    }

    manager.processLine(line);
  }

  return 0;
}
