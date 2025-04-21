#include "Compiler.hpp"
#include "AST.hpp"
#include "Tu4Command.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <fstream>
#include <iterator>
#include <stdexcept>

using namespace ast;

commands_type MT::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t q = 0;
  if (usage_ == Usage::DEFINITION)
    return {};

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    size_t subMaxQ = subCommands.maxQ();

    subCommands.shift(q);
    result.extend(subCommands);

    q += subMaxQ;
  }

  return result;
}

commands_type MT::Lib::to4(const compiler::Alphabet<char> &alphabet) {
	std::ifstream file(strfast() << globals::libDir << "/" << namesTable_[id_] << ".tu4");
	commands_type commands;
	while(!file.eof()) {
		commands.push_back(load<size_t>(file));
	}
	for(auto it = std::begin(commands), itNext = std::next(it); it != std::end(commands); ++it, ++itNext) {
		if(it->isTerm()) {
			commands.erase(it);
			it = std::prev(itNext);
		}
	}

	return commands;
}

commands_type MT::Call::to4(const compiler::Alphabet<char> &alphabet) {
  auto subCommands = definitions_[id_]->to4(alphabet);
  commands_type result = subCommands;
  size_t q0 = subCommands.maxQ();

  for (size_t i = 1; i < pow_; ++i) {
    subCommands.shift(q0);
    result.extend(subCommands);
  }

  return result;
}

commands_type MT::Definition::to4(const compiler::Alphabet<char> &alphabet0) {
  commands_type result;
  auto alphabet = alphabet_->alphabet() || alphabet0;
  size_t q = 0;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    size_t subMaxQ = subCommands.maxQ();

    subCommands.shift(q);
    result.extend(subCommands);

    q += subMaxQ;
  }

  return result;
}

commands_type BeginEnd::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t q = 0;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    size_t subMaxQ = subCommands.maxQ();

    subCommands.shift(q);
    result.extend(subCommands);

    q += subMaxQ;
  }

  return result;
}

commands_type IfFi::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t q = 1;
	std::list<size_t> endStates;
	compiler::Alphabet<char> used;
	for(auto& branch : branches_) {
		result.push_back({tu4::Tu4SetLetter<size_t>{0, branch->letterToCheck(), branch->letterToCheck(), q}});

		auto subCommands = branch->to4(alphabet);
		auto subMaxQ = subCommands.maxQ();
		subCommands.shift(q);

		result.extend(subCommands);
		q += subMaxQ;

		endStates.push_back(q);
		used.insert(branch->letterToCheck());

		++q;
	}
	
	auto notUsed = alphabet / used;
	for(auto& letter : alphabet) {
		for(auto& endState : endStates) 
			result.push_back({tu4::Tu4SetLetter<size_t>(endState, letter, letter, q)});

		if(notUsed.contains(letter))
			result.push_back({tu4::Tu4SetLetter<size_t>(0, letter, letter, q)});
	}
  return result;
}

commands_type IfFi::Branch::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t q = 0;
  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    size_t subMaxQ = subCommands.maxQ();

    subCommands.shift(q);
    result.extend(subCommands);

    q += subMaxQ;
  }

  return result;
}

commands_type DoOd::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t q = 1;

  auto firstBranch = *std::begin(branches_);
  if (firstBranch->isAnyChar() && branches_.size() > 1)
    throw std::logic_error(
        "()!=[some letter] must be unique in DO ... OD circle if exists");

	if(firstBranch->isAnyChar()) {
		auto subCommands = firstBranch->to4(alphabet);
		q += subCommands.maxQ();
		subCommands.shift(1);
		result.extend(subCommands);
		for(auto& letter : alphabet) {
			result.push_back({tu4::Tu4SetLetter<size_t>{q, letter, letter, 0}});
			if(letter == firstBranch->letterToCheck()) 
				continue;
			result.push_back({tu4::Tu4SetLetter<size_t>(0, letter, letter, 1)});
		}
		result.push_back({tu4::Tu4SetLetter<size_t>(0, firstBranch->letterToCheck(), firstBranch->letterToCheck(), q + 1)});
		return result;
	}

  compiler::Alphabet<char> used;
	for(auto& branch : branches_) {
		used.insert(branch->letterToCheck());
		result.push_back({tu4::Tu4SetLetter<size_t>(0, branch->letterToCheck(), branch->letterToCheck(), q)});

		auto subCommands = branch->to4(alphabet);
		auto subMaxQ = subCommands.maxQ();
		subCommands.shift(q);
		result.extend(subCommands);
		q += subMaxQ;

		for(auto& letter : alphabet)
			result.push_back({tu4::Tu4SetLetter<size_t>(q, letter, letter, 0)});

		++q;
	}
	
  auto notUsed = alphabet / used;
	for(auto& letter : notUsed) 
		result.push_back({tu4::Tu4SetLetter<size_t>(0, letter, letter, q)});

  return result;
}

commands_type DoOd::Branch::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
  size_t q = 0;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
    size_t subMaxQ = subCommands.maxQ();

    subCommands.shift(q);
    result.extend(subCommands);

    q += subMaxQ;
  }

  return result;
}

commands_type SetLetter::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;

  for (auto &letter : alphabet)
    result.push_back({tu4::Tu4SetLetter<size_t>{0, letter, letter_, 1}});

  return result;
}

commands_type Tree::to4() {
  compiler::Alphabet<char> alphabet;

  auto commands = root_->to4(alphabet);
	for(auto& c : commands) 
		alphabet.insert(c.letterToCheck());
	
  size_t q = commands.maxQ();
	for(auto& letter : alphabet) 
		commands.push_back({tu4::Tu4SetLetter<size_t>(q, letter, letter, q)});

  return commands;
}
