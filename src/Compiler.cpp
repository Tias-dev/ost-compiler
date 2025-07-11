#include <chrono>
#include "Compiler.hpp"
#include "AST.hpp"
#include "FilePosition.hpp"
#include "Tu4Command.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>

using namespace ast;

static size_t currentState = 0;

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
		std::string filename = strfast() << globals::libDir << namesTable_[id_] << ".tu4";
    commands = compiler::serializer::deserialize(filename, globals::useBinaryFormat);

    for (auto it = std::begin(commands), itNext = std::next(it);
         it != std::end(commands); ++it, ++itNext) {
      if (it->isTerm()) {
        commands.erase(it);
        it = std::prev(itNext);
      }
    }
		cache_ = commands;
  }

  commands.shiftTo(currentState);
  currentState += commands.deltaQ();
  return commands;
}

commands_type MT::Call::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  NodeBase *definition = definitions_[id_];
  definition->begin_ = begin();
  definition->end_ = end();
	if(globals::enableBreakpoints) {
		globals::breakpointer->onEnter(FileRange::fromPositions(begin(), end()));
		size_t qBegin = currentState++; // Dummy state for register mt's call by debugger
		for(auto& letter : alphabet)  
			result.push_back({tu4::Tu4SetLetter<size_t>{qBegin, letter, letter, currentState}});
	} 
		

  for (size_t i = 0; i < pow_; ++i) {
    auto subCommands = definition->to4(alphabet);
    result.extend(subCommands);
  }
	if(globals::enableBreakpoints) 
		globals::breakpointer->onExit();

	return result;
}

commands_type
MT::Definition::to4_impl(const compiler::Alphabet<char> &alphabet0) {
  static std::map<size_t, size_t> defineStack;
  commands_type result;

  if (defineStack.contains(id_)) {
    size_t outQ = defineStack[id_];
    for (auto &letter : alphabet0)
      result.push_back(
          {tu4::Tu4SetLetter<size_t>{currentState, letter, letter, outQ}});
    ++currentState;
    return result;
  }

  defineStack[id_] = currentState;
  if (cache_.has_value()) {
    result = *cache_;
    result.shiftTo(currentState);
    currentState += result.deltaQ();
  } else {
    auto alphabet = alphabet_->alphabet() || alphabet0;

    for (auto &child : childs_) {
      auto subCommands = child->to4(alphabet);
      result.extend(subCommands);
    }
    cache_ = result;
  }

  defineStack.erase(id_);
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
  size_t qBegin = currentState++;
  compiler::Alphabet<char> used{' ', false};
  std::list<size_t> endStates;

  for (auto &branch : ifBranches_) {
    if (branch->isAnyChar()) {
      for (auto &letter : alphabet) { // Set condition check
        if (letter == branch->letterToCheck())
          continue;
        if (used.contains(letter))
          throwBranchRedifinitionError(branch, this, letter);

        result.push_back(
            {tu4::Tu4SetLetter<size_t>{qBegin, letter, letter, currentState}});
        used.insert(letter);
      }
    } else { // Set condition check
      if (used.contains(branch->letterToCheck()))
        throwBranchRedifinitionError(branch, this, branch->letterToCheck());
      used.insert(branch->letterToCheck());

      result.push_back(
          {tu4::Tu4SetLetter<size_t>{qBegin, branch->letterToCheck(),
                                     branch->letterToCheck(), currentState}});
    }
		// Set branch actions
		auto subCommands = branch->to4(alphabet);
		endStates.push_back(currentState++);
		result.extend(subCommands);
  }

  auto notUsed = alphabet / used; 
	if(elseBranch_) {
		for(auto& letter : notUsed) {
			result.push_back({tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, currentState)});
		}
		notUsed.clear();

		auto subCommands = elseBranch_->to4(alphabet);
		endStates.push_back(currentState++);
		result.extend(subCommands);
	} 
		

	// Set exiting after IF ... FI statement
  for (auto &letter : alphabet) {
    for (auto &endState : endStates)
      result.push_back(
          {tu4::Tu4SetLetter<size_t>(endState, letter, letter, currentState)});

    if (notUsed.contains(letter))
      result.push_back(
          {tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, currentState)});
  }
  return result;
}

commands_type DoOd::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t qBegin = currentState++;
  compiler::Alphabet<char> used{'_', false};

  for (auto &branch : branches_) {
    if (branch->isAnyChar()) { // Set condition check
      for (auto &letter : alphabet) {
        if (letter == branch->letterToCheck())
          continue;

        if (used.contains(letter))
          throwBranchRedifinitionError(branch, this, letter);
        used.insert(letter);

        result.push_back(
            {tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, currentState)});
      }
    } else { // Set condition check
      if (used.contains(branch->letterToCheck()))
        throwBranchRedifinitionError(branch, this, branch->letterToCheck());
      used.insert(branch->letterToCheck());

      result.push_back(
          {tu4::Tu4SetLetter<size_t>(qBegin, branch->letterToCheck(),
                                     branch->letterToCheck(), currentState)});
    }
		// Set branch actions
    auto subCommands = branch->to4_impl(alphabet);
    result.extend(subCommands);
		// Set returning to start
    for (auto &letter : alphabet) 
      result.push_back(
          {tu4::Tu4SetLetter<size_t>(currentState, letter, letter, qBegin)});

    ++currentState;
  }

  auto notUsed = alphabet / used; // Set exit for other letters
  for (auto &letter : notUsed)
    result.push_back(
        {tu4::Tu4SetLetter<size_t>(qBegin, letter, letter, currentState)});

  return result;
}

commands_type Branch::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    result.extend(subCommands);
  }

  return result;
}

commands_type ElseBranch::to4_impl(const compiler::Alphabet<char> & alphabet) {
  commands_type result;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    result.extend(subCommands);
  }

  return result;
}

commands_type SetLetter::to4_impl(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t qBegin = currentState++;

  for (auto &letter : alphabet)
    result.push_back(
        {tu4::Tu4SetLetter<size_t>{qBegin, letter, letter_, currentState}});

  return result;
}

commands_type Tree::to4() {
	currentState = 0;
  compiler::Alphabet<char> alphabet;

  auto commands = root_->to4_impl(alphabet);
  for (auto &c : commands)
    alphabet.insert(c.letterToCheck());

	logger::debug() << "Terminal state: " << currentState;

  for (auto &letter : alphabet)
    commands.push_back({tu4::Tu4SetLetter<size_t>(currentState, letter, letter,
                                                  currentState)});

  return commands;
}

class MoveMT : public NodeBase {
  tu4::MoveDirection dir_;
  void init(token::tokens_list &tokens) override {}


public:
  MoveMT(tu4::MoveDirection dir)
      : NodeBase(ExprType::MT), dir_(dir) {}
  std::string toString() override {
    return (dir_ == tu4::MoveDirection::LEFT ? "<" : ">");
  }

  commands_type to4_impl(const compiler::Alphabet<char> &alphabet) override {
    commands_type result;
    size_t qBegin = currentState++;
    for (auto &letter : alphabet)
      result.push_back(
          {tu4::Tu4Move<size_t>{qBegin, letter, dir_, currentState}});

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

void compileProgram(const std::string &fileName, const std::string &libDir,
                    const std::string &outDir, bool useBinaryFormat,
                    bool enableBreakpoints) {
	logger::debug() << "Compiling: " << fileName;
	if(enableBreakpoints) 
		globals::breakpointer = std::make_shared<FileBreakpointer>();
	ast::Tree::clearNamesTable();
	FilePosition::fileCodesBimap().clear();
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
		out << "Tokenizing time:" << std::chrono::duration_cast<duration_t>(end - start).count() << DURATION_SUFFIX;
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
		out << "AST creating time:" << std::chrono::duration_cast<duration_t>(end - start).count() << DURATION_SUFFIX;
  }
	std::string foutName = strfast() << outDir << astTree.getTreeName() << ".tu4";

	compiler::commands_type  commands;
	{
		auto start = std::chrono::system_clock::now();
		commands = astTree.to4();
		auto end = std::chrono::system_clock::now();
		logger::debug() << "Compiling from AST to mt4 form time: " << std::chrono::duration_cast<duration_t>(end - start).count() << DURATION_SUFFIX;
	}
	{
		auto start = std::chrono::system_clock::now();
		compiler::serializer::serialize(commands, FilePosition::fileCodesBimap(), foutName, useBinaryFormat);
		auto end = std::chrono::system_clock::now();
		logger::debug() << "Serializing compiled program time: " << std::chrono::duration_cast<duration_t>(end - start).count() << DURATION_SUFFIX;
	}
	auto end_ts = std::chrono::system_clock::now();
	logger::debug() << "Total compiling time: " << std::chrono::duration_cast<duration_t>(end_ts - start_ts).count() << DURATION_SUFFIX;
	logger::info() << "Commands written to file: " << foutName;
}
