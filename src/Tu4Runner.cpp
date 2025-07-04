#include "Tu4Runner.hpp"
#include "BreakpointManager.hpp"
#include "globals.hpp"
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>


std::unique_ptr<tu4run::Tu4Runner<size_t, char>> tu4run::initRunner(const std::string &fileName,
                                    const std::string &line) {
  std::ifstream file(fileName);
  if (!file.is_open())
    throw std::invalid_argument("Bad file name");

  auto commandsRaw = loadMultiple<size_t, char>(file);
  compiler::Commands<size_t, char> commands;
  for (auto &command : commandsRaw)
    commands.push_back(command);

  tu4run::Line<char> line_{line.c_str()};
  auto runner = std::make_unique<tu4run::Tu4Runner<size_t, char>>(line_, commands);

  return runner;
}
std::tuple<std::unique_ptr<tu4run::Tu4Runner<size_t, char>>, tu4run::Tu4RunnerBreakpoints> tu4run::initRunnerWithBreakpoints(const std::string &fileName,
                                    const std::string &line) {
  std::ifstream file(fileName);
  if (!file.is_open())
    throw std::invalid_argument("Bad file name");
	std::string templine;
	std::getline(file, templine);
	if(templine != globals::debugFirstLine) {
		throw std::invalid_argument(strfast() << "Given program not compiled with -g (debug) flag");
	}

	auto commandsRaw = loadMultiple<size_t, char>(file);
  compiler::Commands<size_t, char> commands;
  for (auto &command : commandsRaw)
    commands.push_back(command);

  tu4run::Line<char> line_{line.c_str()};
  auto runner = std::make_unique<tu4run::Tu4Runner<size_t, char>>(line_, commands);
	auto stateBreakpointer = std::make_unique<StateBreakpointManager<size_t>>();
	auto lineBreakpointer = std::make_unique<LineBreakpointManager<size_t>>();

	Tu4RunnerBreakpoints breakpoints{.stateBreakpoints = stateBreakpointer->breakpoints(), .lineBreakpoints = lineBreakpointer->breakpoints()};
	runner->addBreakpointManager(std::move(stateBreakpointer));
	runner->addBreakpointManager(std::move(lineBreakpointer));

  return {std::move(runner), breakpoints};
}
