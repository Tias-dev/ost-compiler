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
    std::ifstream file(strfast()
                       << globals::libDir << namesTable_[id_] << ".tu4");
    if (!file.is_open())
      throw std::invalid_argument(strfast()
                                  << globals::libDir << namesTable_[id_]
                                  << ".tu4" << " not found!");
    auto commandsList = loadMultiple<size_t>(file);
    for (auto &command : commandsList)
      commands.push_back(command);

    for (auto it = std::begin(commands), itNext = std::next(it);
         it != std::end(commands); ++it, ++itNext) {
      if (it->isTerm()) {
        commands.erase(it);
        it = std::prev(itNext);
      }
    }
  }

  cache_ = commands;
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
		globals::breakpointer->onEnter(begin(), end());
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
  compiler::Alphabet<char> alphabet;

  auto commands = root_->to4_impl(alphabet);
  for (auto &c : commands)
    alphabet.insert(c.letterToCheck());

  if (globals::printDebugInfo)
    std::cout << "Terminal state: " << currentState << std::endl;

  for (auto &letter : alphabet)
    commands.push_back({tu4::Tu4SetLetter<size_t>(currentState, letter, letter,
                                                  currentState)});

  return commands;
}

class MoveMT : public NodeBase {
  tu4::MoveDirection dir_;
  void init(token::tokens_list &tokens) override {}

  const static FilePosition defaultFpos;

public:
  MoveMT(tu4::MoveDirection dir)
      : NodeBase(ExprType::MT, defaultFpos, defaultFpos), dir_(dir) {}
  std::string toString() override {
    return (dir_ == tu4::MoveDirection::LEFT ? "<" : ">");
  }

  commands_type to4_impl(const compiler::Alphabet<char> &alphabet) override {
    commands_type result;
    size_t qBegin = currentState++;
    for (auto &letter : alphabet)
      result.push_back(
          {tu4::Tu4Move<size_t>{qBegin, letter, dir_, currentState}});

    {
      logger::debug out;
      out << "MT: " << toString() << " Alphabet: ";
      for (auto &letter : alphabet)
        out << '[' << letter << "], ";
    }

    return result;
  }
  commands_type to4(const compiler::Alphabet<char> &alphabet) override {
    return to4_impl(alphabet);
  }
};

const FilePosition MoveMT::defaultFpos =
    FilePosition(std::make_shared<std::string>("default fpos"), 0, 0);

std::map<size_t, NodeBase *> MT::definitions_{
    {0, new MoveMT{tu4::MoveDirection::LEFT}},
    {1, new MoveMT{tu4::MoveDirection::RIGHT}}};
