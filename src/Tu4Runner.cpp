#include "Tu4Runner.hpp"
#include <fstream>


tu4run::Tu4Runner<size_t, char> *tu4run::initRunner(const std::string &fileName,
                                    const std::string &line) {
  std::ifstream file(fileName);
  if (!file.is_open())
    throw std::invalid_argument("Bad file name");

  auto commandsRaw = loadMultiple<size_t, char>(file);
  compiler::Commands<size_t, char> commands;
  for (auto &command : commandsRaw)
    commands.push_back(command);

  tu4run::Line<char> line_{line.c_str()};
  auto runner = new tu4run::Tu4Runner<size_t, char>(line_, commands);

  return runner;
}
