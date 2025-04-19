#include "Compiler.hpp"
#include "AST.hpp"
#include "Tu4Command.hpp"
#include <codecvt>
#include <iterator>
#include <stdexcept>

using namespace ast;

commands_type MT::to4(const compiler::Alphabet<char> &alphabet) {
  commands_type result;
	size_t q = 0;
	if(usage_ == Usage::DEFINITION) 
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
  // ------------------------------------------------------------
  // |  There must be loading tu4 commands from external file  |
  // ------------------------------------------------------------

  throw std::logic_error("Not realized yet");
}

commands_type MT::Call::to4(const compiler::Alphabet<char> &alphabet) {
	auto subCommands = definitions_[id_]->to4(alphabet);
	commands_type result = subCommands;
	size_t q0 = subCommands.maxQ();

  for(size_t i = 1; i < pow_; ++i) {
		subCommands.shift(q0);
		result.extend(subCommands);
  }

	return result;
}

commands_type
MT::Definition::to4(const compiler::Alphabet<char> &alphabet0) {
  commands_type result;
  auto alphabet = compiler::Alphabet(alphabet_->alphabet(), alphabet0);
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
	size_t q = 0;

	result.push_back({tu4::Tu4SetLetter<size_t>{0, 'T', 'T', 1}});
	auto trueSubCommands = true_->to4(alphabet);
	trueSubCommands.shift(1);
	q = trueSubCommands.maxQ();

	result.push_back({tu4::Tu4SetLetter<size_t>{0, 'F', 'F', q }});
	auto falseSubCommands = true_->to4(alphabet);
	trueSubCommands.shift(q);

  return result;
}

commands_type
IfFi::Branch::to4(const compiler::Alphabet<char> &alphabet) {
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
  size_t q = 0;
  for (auto &branch : branches_) {
		auto subCommands = branch->to4(alphabet);
		size_t subMaxQ = subCommands.maxQ() + 1;

		subCommands.shift(q);

		result.extend(subCommands);
		if(branch->isAnyChar()) {
			result.push_back({tu4::Tu4SetLetter<size_t>{0, branch->letterToCheck(), branch->letterToCheck(), q + subMaxQ}});
		} else {
			result.push_back({tu4::Tu4SetLetter<size_t>{0, branch->letterToCheck(), branch->letterToCheck(), q}});
			q += subMaxQ;
		}
	}

  return result;
}

commands_type
DoOd::Branch::to4(const compiler::Alphabet<char> &alphabet) {
	commands_type result;
	size_t q = 0;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet);
		size_t subMaxQ = subCommands.maxQ();

		subCommands.shift(q);
		result.extend(subCommands);

		q += subMaxQ;
  }

	for(auto& letter : alphabet)
		result.push_back({tu4::Tu4SetLetter<size_t>{q, letter, letter, 0}});

  return result;
}

commands_type
SetLetter::to4(const compiler::Alphabet<char> &alphabet) {
	commands_type result;

	for(auto& letter : alphabet) 
		result.push_back({tu4::Tu4SetLetter<size_t>{0, letter, letter_, 1}});
	
	return result;
}

commands_type
Tree::to4() {
	compiler::Alphabet<char> alphabet;

	auto commands = root_->forceTo4(alphabet);
	size_t q = commands.maxQ();
	commands.push_back({tu4::Tu4SetLetter<size_t>{q, '_', '_', q}});
	for(auto head = std::begin(commands); head != std::end(commands); ++head) {
		for(auto tail = std::begin(commands); tail != head; ++tail) {
			if(
					head->q0() == tail->q0() &&
					head->letterToCheck() == tail->letterToCheck()
					) {
				auto tail0 = std::prev(tail);
				commands.erase(tail);
				tail = tail0;
			}
		}
	}

	return commands;
}
