#include "Compiler.hpp"
#include "AST.hpp"
#include "FilePosition.hpp"
#include "Tu4Command.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <chrono>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>

using namespace ast;

class StateManager {
  size_t currentState_ = 0;

public:
  void reset() { currentState_ = 0; };
  // if first call or after reset() return 0
  // else return value returned by last nextState() function call
  size_t currentState() const { return currentState_; }

  // if nextState not locked return new uniq state
  // else return locked nextState and unlock it
  size_t nextState() { return ++currentState_; }

  void shift(long amount) { currentState_ += amount; }
};

static StateManager states;

commands_type MT::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  switch (usage_) {
  case Usage::DEFINITION:
  case Usage::LIB:
    return {};
  default:
    break;
  };

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    result.extend(subCommands);
  }

  return result;
}

commands_type MT::Lib::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type commands;
  if (cache_.has_value()) {
    commands = *cache_;
  } else {
    std::string filename = strfast()
                           << globals::libDir << namesTable_[id_] << ".tu4";
    commands =
        compiler::serializer::deserialize(filename, globals::useBinaryFormat);

    for (auto it = std::begin(commands), itNext = std::next(it);
         it != std::end(commands); ++it, ++itNext) {
      if (it->isTerm()) {
        commands.erase(it);
        it = std::prev(itNext);
      }
    }
    cache_ = commands;
  }

  commands.shiftTo(states.currentState());
  states.shift(commands.deltaQ());
  return commands;
}

commands_type MT::Call::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  NodeBase *definition = definitions_[id_];
  definition->begin_ = begin();
  definition->end_ = end();
  if (globals::enableBreakpoints) {
    globals::breakpointer->onEnter(FileRange::fromPositions(begin(), end()));
    size_t qCurrent = states.currentState(), qNext = states.nextState();
    for (auto &letter :
         alphabet) // Dummy state for register mt's call by debugger
      result.push_back(
          {tu4::Tu4SetLetter<size_t>{qCurrent, letter, letter, qNext}});
  }

  for (size_t i = 0; i < pow_; ++i) {
    auto subCommands = definition->to4(alphabet);
    result.extend(subCommands);
  }

  if (globals::enableBreakpoints)
    globals::breakpointer->onExit();

  return result;
}

commands_type
MT::Definition::to4_impl(const compiler::Alphabet<char> &alphabet0) {
  static std::map<size_t, size_t> definedMT;
  commands_type result;

  if (definedMT.contains(id_)) {
    size_t outQ = definedMT[id_];
    for (auto &letter : alphabet0)
      result.push_back({tu4::Tu4SetLetter<size_t>{states.currentState(), letter,
                                                  letter, outQ}});
    states.nextState();
    return result;
  }

  definedMT[id_] = states.currentState();
  if (cache_.has_value()) {
    result = *cache_;
    result.shiftTo(states.currentState());
    states.shift(result.deltaQ());
  } else {
    auto alphabet = alphabet_->alphabet() || alphabet0;

    for (auto &child : childs_) {
      auto subCommands = child->to4(alphabet);
      result.extend(subCommands);
    }
    cache_ = result;
  }

  definedMT.erase(id_);
  return result;
}

commands_type BeginEnd::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    result.extend(subCommands);
  }

  return result;
}

void throwBranchRedifinitionError(Branch *branch, NodeBase *statement,
                                  char letter) {
  throw std::logic_error(strfast()
                         << "Redefinition in " << branch->toString() << " for ["
                         << letter << "] in [" << statement->toString() << " : "
                         << statement->begin().to_string() << "] statement");
}

commands_type IfFi::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t qBegin = states.currentState();
  compiler::Alphabet<char> used{' ', false};
  std::list<size_t> endStates;

  for (auto &branch : ifBranches_) {
    size_t qBranchBegin = states.nextState();
    if (branch->isAnyChar()) {
      for (auto &letter : alphabet) { // Set condition check
        if (letter == branch->letterToCheck())
          continue;
        if (used.contains(letter))
          throwBranchRedifinitionError(branch, this, letter);

        result.push_back(
            {tu4::Tu4SetLetter<size_t>{qBegin, letter, letter, qBranchBegin}});
        used.insert(letter);
      }
    } else { // Set condition check
      if (used.contains(branch->letterToCheck()))
        throwBranchRedifinitionError(branch, this, branch->letterToCheck());
      used.insert(branch->letterToCheck());

      result.push_back(
          {tu4::Tu4SetLetter<size_t>{qBegin, branch->letterToCheck(),
                                     branch->letterToCheck(), qBranchBegin}});
    }
    // Set branch actions
    auto subCommands = branch->to4(alphabet);
    endStates.push_back(states.currentState());
    result.extend(subCommands);
  }

  auto notUsed = alphabet / used;
  if (elseBranch_) {
    size_t qBranchBegin = states.nextState();
    for (auto &letter : notUsed) 
      result.push_back(
          {tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, qBranchBegin)});
    notUsed.clear();

    auto subCommands = elseBranch_->to4(alphabet);
    endStates.push_back(states.currentState());
    result.extend(subCommands);
  }

  size_t qEnd = states.nextState();
  // Set exiting after IF ... FI statement
  for (const auto &endState : endStates)
		result.overrideEndState(endState, qEnd);
	result.executeOverrides();

	// handle other letters
  for (const auto &letter : notUsed)
    result.push_back({tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, qEnd)});

  return result;
}

commands_type DoOd::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t qBegin = states.currentState();
  compiler::Alphabet<char> used{'_', false};

  for (auto &branch : branches_) {
    size_t qBranchBegin = states.nextState();
    if (branch->isAnyChar()) { // Set condition check
      for (auto &letter : alphabet) {
        if (letter == branch->letterToCheck())
          continue;

        if (used.contains(letter))
          throwBranchRedifinitionError(branch, this, letter);
        used.insert(letter);

        result.push_back(
            {tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, qBranchBegin)});
      }
    } else { // Set condition check
      if (used.contains(branch->letterToCheck()))
        throwBranchRedifinitionError(branch, this, branch->letterToCheck());
      used.insert(branch->letterToCheck());

      result.push_back(
          {tu4::Tu4SetLetter<size_t>(qBegin, branch->letterToCheck(),
                                     branch->letterToCheck(), qBranchBegin)});
    }
    // Set branch actions
    auto subCommands = branch->to4_impl(alphabet);
    result.extend(subCommands);
		result.overrideEndState(states.currentState(), qBegin);
  }

  size_t qEnd = states.nextState();
	result.executeOverrides();
	
  auto notUsed = alphabet / used; // Set exit for other letters
  for (auto &letter : notUsed)
    result.push_back({tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, qEnd)});

  if (notUsed.empty())
    logger::warning()
        << "In " << begin_.to_string()
        << " detected DO ... OD loop for all existing letters in alphabet. "
           "This will lead to infinity loop, you probably don't want this";

  return result;
}

commands_type Branch::to4_impl(const compiler::Alphabet<char> &alphabet) {
  if (childs_.empty())
    return {};

  commands_type result;
  for (const auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    result.extend(subCommands);
  }
  return result;
}

commands_type ElseBranch::to4_impl(const compiler::Alphabet<char> &alphabet) {
  if (childs_.empty())
    return {};

  commands_type result;
  for (const auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    result.extend(subCommands);
  }
  return result;
}

commands_type SetLetter::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t qBegin = states.currentState(), qEnd = states.nextState();

  for (auto &letter : alphabet)
    result.push_back(
        {tu4::Tu4SetLetter<size_t>{qBegin, letter, letter_, qEnd}});

  return result;
}

commands_type Tree::to4() {
  states.reset();
  compiler::Alphabet<char> alphabet;

  auto commands = root_->to4_impl(alphabet);
  for (auto &c : commands)
    alphabet.insert(c.letterToCheck());

  size_t qTerm = states.currentState();
  logger::debug() << "Terminal state: " << qTerm;

  for (auto &letter : alphabet)
    commands.push_back(
        {tu4::Tu4SetLetter<size_t>(qTerm, letter, letter, qTerm)});

  return commands;
}

class MoveMT : public NodeBase {
  tu4::MoveDirection dir_;
  void init(token::tokens_list &tokens) override {}

public:
  MoveMT(tu4::MoveDirection dir) : NodeBase(ExprType::MT), dir_(dir) {}
  std::string toString() override {
    return (dir_ == tu4::MoveDirection::LEFT ? "<" : ">");
  }

  commands_type to4_impl(const compiler::Alphabet<char> &alphabet) override {
    commands_type result;
    size_t qBegin = states.currentState(), qEnd = states.nextState();
    for (auto &letter : alphabet)
      result.push_back({tu4::Tu4Move<size_t>{qBegin, letter, dir_, qEnd}});

    return result;
  }
  commands_type to4(const compiler::Alphabet<char> &alphabet) override {
    return to4_impl(alphabet);
  }
};

std::map<size_t, NodeBase *> MT::definitions_{
    {0, new MoveMT{tu4::MoveDirection::LEFT}},
    {1, new MoveMT{tu4::MoveDirection::RIGHT}}};

using duration_t = std::chrono::milliseconds;
#define DURATION_SUFFIX "ms";

std::pair<commands_type, compiler::mt_name_t>
compiler::compileProgram(const std::string &fileName, const std::string &libDir,
                         bool useBinaryFormat, bool enableBreakpoints,
                         bool printDebugInfo) {
  logger::debug() << "Compiling: " << fileName;
  if (enableBreakpoints)
    globals::breakpointer = std::make_shared<FileBreakpointer>();
  ast::Tree::clearNamesTable();
  FilePosition::fileCodesBimap().clear();
  std::fstream file(fileName);
  if (!file.is_open())
    throw std::invalid_argument(strfast() << "Can't open file: " << fileName);

  CharStream stream(file);

  auto tokenizer = token::Tokenizer{};
  FileRoller roller{std::make_shared<std::string>(fileName)};
  auto tokens = tokenizer.parse(stream, roller);
  file.close();

  if (printDebugInfo) {
    auto start = std::chrono::system_clock::now();
    logger::debug out;
    if (globals::verboseOutput) {
      out << "Tokenizer output:" << std::endl;
      for (auto &token : tokens)
        out << token.toString() << std::endl;
      out << "-----------------------------" << std::endl << std::endl;
    }
    auto end = std::chrono::system_clock::now();
    out << "Tokenizing time:"
        << std::chrono::duration_cast<duration_t>(end - start).count()
        << DURATION_SUFFIX;
  }

  ast::Tree astTree{tokens, fileName};
  if (printDebugInfo) {
    auto start = std::chrono::system_clock::now();
    logger::debug out;
    if (globals::verboseOutput) {
      out << "Created AST tree:" << std::endl;
      astTree.print(out);
      out << "-----------------------------" << std::endl << std::endl;
      out << "Names table:" << std::endl;
      ast::MT::printNamesTable(out);
    }
    auto end = std::chrono::system_clock::now();
    out << "AST creating time:"
        << std::chrono::duration_cast<duration_t>(end - start).count()
        << DURATION_SUFFIX;
  }
  compiler::commands_type commands;
  auto start = std::chrono::system_clock::now();
  commands = astTree.to4();
  if (printDebugInfo) {
    auto end = std::chrono::system_clock::now();
    logger::debug()
        << "Compiling from AST to mt4 form time: "
        << std::chrono::duration_cast<duration_t>(end - start).count()
        << DURATION_SUFFIX;
  }
  return {commands, astTree.getTreeName()};
}

void compileAndSaveProgram(const std::string &fileName,
                           const std::string &libDir, const std::string &outDir,
                           bool useBinaryFormat, bool enableBreakpoints,
                           bool printDebugInfo) {
  auto start_ts = std::chrono::system_clock::now();
  auto [commands, mt_name] = compiler::compileProgram(
      fileName, libDir, useBinaryFormat, enableBreakpoints, printDebugInfo);
  std::string foutName = strfast() << outDir << mt_name << ".tu4";
  {
    auto start = std::chrono::system_clock::now();
    compiler::serializer::serialize(commands, FilePosition::fileCodesBimap(),
                                    foutName, useBinaryFormat);
    auto end = std::chrono::system_clock::now();
    if (printDebugInfo)
      logger::debug()
          << "Serializing compiled program time: "
          << std::chrono::duration_cast<duration_t>(end - start).count()
          << DURATION_SUFFIX;
  }
  auto end_ts = std::chrono::system_clock::now();
  if (printDebugInfo)
    logger::debug()
        << "Total compiling time: "
        << std::chrono::duration_cast<duration_t>(end_ts - start_ts).count()
        << DURATION_SUFFIX;
  logger::info() << "Commands written to file: " << foutName;
}
