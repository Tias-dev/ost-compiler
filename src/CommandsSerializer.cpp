#ifndef COMMANDS_SERIALIZER_HPP_
#define COMMANDS_SERIALIZER_HPP_
#include "Compiler.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "FilePosition.hpp"
#include "Tu4Command.hpp"
#include "globals.hpp"
#include "utils.hpp"

namespace compiler::serializer {
struct SerializePreambule {
  bool isBreakpointsEnabled;
  size_t nUsedFiles;
  size_t nCommands;
};

struct FileNameEntry {
  size_t code;
  size_t strlen;
  const char *str;
};

struct CommandEntry {
  SIZE_T q0, q;
  char checkedLetter, action;
};

struct DebugCommandEntry : public CommandEntry {
  FileRange range;
};

CommandEntry toEntry(const tu4::tu4_union<SIZE_T, char> &command) {
  CommandEntry result{.q0 = command.q0(),
                      .q = command.q(),
                      .checkedLetter = command.letterToCheck()};
  if (command.isSetLetter())
    result.action = command.getLetterToSet();
  else
    result.action =
        (command.getMoveDirection() == tu4::MoveDirection::LEFT ? '<' : '>');

  return result;
}

void serialize(const commands_type &commands,
               const bimap<std::string, SIZE_T> &fileCodes,
               const std::string &outFileName, bool useBinaryFormat) {
  if (!useBinaryFormat) {
    std::ofstream fout(outFileName);
    if (!fout.is_open())
      throw std::invalid_argument(strfast()
                                  << "Can't open file [" << outFileName << "]");
    if (globals::enableBreakpoints) {
      fout << globals::debugFirstLine
           << "\n// Please not modify comments in program below\n";
      const auto &fileCodeEntries = fileCodes.backward();
      for (const auto &entry : fileCodeEntries)
        fout << "// " << entry.first << ' ' << entry.second << '\n';
    }
    for (auto &command : commands)
      fout << command << '\n';
    return;
  }
  SerializePreambule preambule{.isBreakpointsEnabled =
                                   globals::enableBreakpoints,
                               .nUsedFiles = fileCodes.size(),
                               .nCommands = commands.size()};
  std::vector<FileNameEntry> fileNameEntries;
  const auto &fileCodesData = fileCodes.forward();
  for (const auto &fileEntry : fileCodesData)
    fileNameEntries.push_back(FileNameEntry{.code = fileEntry.second,
                                            .strlen = fileEntry.first.size(),
                                            .str = fileEntry.first.c_str()});

  FILE *fout = fopen(outFileName.c_str(), "wb");
  if (!fout)
    throw std::invalid_argument(strfast()
                                << "Can't open file [" << outFileName << "]");
  fwrite(&preambule, sizeof(preambule), 1, fout);
  for (const auto &fileEntry : fileNameEntries) {
    fwrite(&fileEntry.code, sizeof(fileEntry.code), 1, fout);
    fwrite(&fileEntry.strlen, sizeof(fileEntry.strlen), 1, fout);
    fwrite(fileEntry.str, sizeof(char), fileEntry.strlen, fout);
  }

  if (globals::enableBreakpoints) {
    std::vector<DebugCommandEntry> commandEntries;
    for (const auto &command : commands)
      commandEntries.push_back(
          DebugCommandEntry{toEntry(command), *command.debugBreakpoint()});

    fwrite(commandEntries.data(), sizeof(DebugCommandEntry),
           preambule.nCommands, fout);
  } else {
    std::vector<CommandEntry> commandEntries;
    for (const auto &command : commands)
      commandEntries.push_back(toEntry(command));

    fwrite(commandEntries.data(), sizeof(CommandEntry), preambule.nCommands,
           fout);
  }
  fclose(fout);
}

commands_type deserialize(const std::string &fileName, bool useBinaryFormat) {
  commands_type commands;
  auto &globalFileCodesBimap = FilePosition::fileCodesBimap();
  bimap<std::string, SIZE_T> fileCodesBimap;
  std::map<SIZE_T, SIZE_T> newToOldFileCodes;

  if (!useBinaryFormat) {
    {
      std::ifstream fin(fileName);
      std::string line, temp;
      SIZE_T code, indexBegin = 0, indexEnd;
      std::getline(fin, line);
      if (globals::enableBreakpoints && line != globals::debugFirstLine)
        throw std::logic_error(strfast()
                               << "Trying to load file [" << fileName
                               << "] compiled without debug support(-g) to "
                                  "program that needs debug support!");

      if (line == globals::debugFirstLine)
        globals::enableBreakpoints = true;
      std::getline(fin, line);

      while (std::getline(fin, line) && line.starts_with("//")) {
        while (!isdigit(line[indexBegin]))
          ++indexBegin;
        indexEnd = indexBegin;
        while (isdigit(line[indexEnd]))
          ++indexEnd;
        code = std::atoll(line.substr(indexBegin, indexEnd).c_str());
        while (line[indexEnd++] != ' ') {
        }

        auto fileName = line.substr(indexEnd);
        fileCodesBimap.add(fileName, code);
      }
    }
    std::ifstream fin(fileName);
    if (!fin.is_open())
      throw std::invalid_argument(strfast()
                                  << "Can't open file [" << fileName << "]");
    auto commandsRaw = loadMultiple<SIZE_T, char>(fin);
    for (const auto &command : commandsRaw)
      commands.push_back(command);
  } else {
    SerializePreambule preambule;
    FileNameEntry fileNameEntry;

    FILE *fin = fopen(fileName.c_str(), "rb");
    if (!fin)
      throw std::invalid_argument(strfast()
                                  << "Can't open file [" << fileName << "]");
    fread(&preambule, sizeof(preambule), 1, fin);
    SIZE_T buffsize = 100;
    char *buffer = (char *)malloc(sizeof(char) * buffsize);
    for (SIZE_T i = 0; i < preambule.nUsedFiles; ++i) {
      fread(&fileNameEntry.code, sizeof(fileNameEntry.code), 1, fin);
      fread(&fileNameEntry.strlen, sizeof(fileNameEntry.strlen), 1, fin);

      if (fileNameEntry.strlen > buffsize) {
        buffsize = fileNameEntry.strlen + 1;
        buffer = (char *)realloc(buffer, buffsize);
      }

      fread(buffer, sizeof(char), fileNameEntry.strlen, fin);
      buffer[fileNameEntry.strlen] = 0;
      std::string fileName{buffer};
      fileCodesBimap.add(fileName, fileNameEntry.code);
    }
    free(buffer);
    if (preambule.isBreakpointsEnabled) {
      std::vector<DebugCommandEntry> commandEntries(preambule.nCommands);
      fread(commandEntries.data(), sizeof(DebugCommandEntry),
            preambule.nCommands, fin);
      for (const auto &entry : commandEntries) {
        if (entry.action == '<') {
          tu4::Tu4Move<SIZE_T, char> command{entry.q0, entry.checkedLetter,
                                             tu4::MoveDirection::LEFT, entry.q,
                                             false};
          command.setBreakpoint(entry.range);
          commands.emplace_back(command);
        } else if (entry.action == '>') {
          tu4::Tu4Move<SIZE_T, char> command{entry.q0, entry.checkedLetter,
                                             tu4::MoveDirection::RIGHT, entry.q,
                                             false};
          command.setBreakpoint(entry.range);
          commands.emplace_back(command);
        } else {
          tu4::Tu4SetLetter<SIZE_T, char> command{entry.q0, entry.checkedLetter,
                                                  entry.action, entry.q, false};
          command.setBreakpoint(entry.range);
          commands.emplace_back(command);
        }
      }
    } else {
      if (globals::enableBreakpoints)
        throw std::logic_error(strfast()
                               << "Trying to load file [" << fileName
                               << "] compiled without debug support(-g) to "
                                  "program that needs debug support!");
      std::vector<CommandEntry> commandEntries(preambule.nCommands);
      fread(commandEntries.data(), sizeof(CommandEntry), preambule.nCommands,
            fin);
      for (const auto &entry : commandEntries) {
        if (entry.action == '<') {
          tu4::Tu4Move<SIZE_T, char> command{entry.q0, entry.checkedLetter,
                                             tu4::MoveDirection::LEFT, entry.q,
                                             false};
          commands.push_back({command});
        } else if (entry.action == '>') {
          tu4::Tu4Move<SIZE_T, char> command{entry.q0, entry.checkedLetter,
                                             tu4::MoveDirection::RIGHT, entry.q,
                                             false};
          commands.push_back({command});
        } else {
          tu4::Tu4SetLetter<SIZE_T, char> command{entry.q0, entry.checkedLetter,
                                                  entry.action, entry.q, false};
          commands.push_back({command});
        }
      }
    }
    fclose(fin);
  }

  if (std::begin(commands)->debugBreakpoint().has_value()) {
    const auto &fileEntries = fileCodesBimap.forward();
    for (const auto &fileEntry : fileEntries) {
      if (!globalFileCodesBimap.contains(fileEntry.first))
        globalFileCodesBimap.add(fileEntry.first, globalFileCodesBimap.size());
      newToOldFileCodes[fileEntry.second] =
          globalFileCodesBimap[fileEntry.first];
    }

    for (auto &command : commands) {
      auto debugBreakpoint = *command.debugBreakpoint();
      debugBreakpoint.fileCode = newToOldFileCodes[debugBreakpoint.fileCode];
      command.setBreakpoint(debugBreakpoint);
    }
  }

  return commands;
}
} // namespace compiler::serializer
#endif // !COMMANDS_SERIALIZER_HPP_
