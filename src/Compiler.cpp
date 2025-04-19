#include "AST.hpp"
#include "Tu4Command.hpp"
#include <stdexcept>

using namespace ast;

std::list<tu4::tu4_union<size_t>> MT::to4(const std::set<char> &alphabet,
                                          size_t &q0) {
  std::list<tu4::tu4_union<size_t>> result;
  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet, q0);
    for (auto &subCommand : subCommands)
      result.push_back(subCommand);
  }

  return result;
}

std::list<tu4::tu4_union<size_t>> MT::Lib::to4(const std::set<char> &alphabet,
                                               size_t &q0) {
  // ------------------------------------------------------------
  // |  There must be loading tu4 commands from external files  |
  // ------------------------------------------------------------

  throw std::logic_error("Not realized yet");
}

std::list<tu4::tu4_union<size_t>> MT::Call::to4(const std::set<char> &alphabet,
                                                size_t &q0) {
  return definitons_[id_]->to4(alphabet, q0);
}

std::list<tu4::tu4_union<size_t>>
MT::Definition::to4(const std::set<char> &alphabet0, size_t &q0) {
  std::list<tu4::tu4_union<size_t>> result;
  std::set<char> alphabet = alphabet_->alphabet();
  alphabet.insert(std::begin(alphabet0), std::end(alphabet0));

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet, q0);
    for (auto &subCommand : subCommands)
      result.push_back(subCommand);
  }

  return result;
}

std::list<tu4::tu4_union<size_t>> BeginEnd::to4(const std::set<char> &alphabet0,
                                                size_t &q0) {
  std::list<tu4::tu4_union<size_t>> result;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet0, q0);
    for (auto &subCommand : subCommands)
      result.push_back(subCommand);
  }

  return result;
}

std::list<tu4::tu4_union<size_t>> IfFi::to4(const std::set<char> &alphabet,
                                            size_t &q0) {
  std::list<tu4::tu4_union<size_t>> result;
  size_t maxQ = q0;
  for (auto &child : childs_) {
    size_t curQ0 = q0;
    auto subCommands = child->to4(alphabet, curQ0);
    for (auto &subCommand : subCommands)
      result.push_back(subCommand);

    maxQ = std::max(maxQ, curQ0);
  }

  q0 = maxQ;
  return result;
}

std::list<tu4::tu4_union<size_t>>
IfFi::Branch::to4(const std::set<char> &alphabet, size_t &q0) {
  std::list<tu4::tu4_union<size_t>> result;
  result.push_back(
      tu4::Tu4SetLetter<size_t>(q0, letterToCheck_, letterToCheck_, q0 + 1));
  q0 = q0 + 1;

  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet, q0);
    for (auto &subCommand : subCommands)
      result.push_back(subCommand);
  }

  return result;
}

std::list<tu4::tu4_union<size_t>> DoOd::to4(const std::set<char> &alphabet,
                                            size_t &q0) {
  std::list<tu4::tu4_union<size_t>> result;
  size_t maxQ = q0;
  for (auto &child : childs_) {
    size_t curQ = q0;
    auto subCommands = child->to4(alphabet, curQ);
    for (auto &subCommand : subCommands)
      result.push_back(subCommand);

    maxQ = std::max(maxQ, curQ);
  }
  q0 = maxQ;
  return result;
}

std::list<tu4::tu4_union<size_t>>
DoOd::Branch::to4(const std::set<char> &alphabet, size_t &q0) {
  std::list<tu4::tu4_union<size_t>> result;
  size_t maxQ = q0;

  if (isAnyChar_) {
		for(auto& letter : alphabet) {
			if(letter == letterToCheck_) 
				continue;
			maxQ = std::max(maxQ, to4_impl(result, letter, alphabet, q0));
		}
		result.push_back(tu4::Tu4SetLetter<size_t>(q0, letterToCheck_, letterToCheck_, maxQ));
  } else {
    maxQ = std::max(maxQ, to4_impl(result, letterToCheck_, alphabet, q0));
  }

  return result;
}

size_t DoOd::Branch::to4_impl(std::list<tu4::tu4_union<size_t>> &commands,
                              char letter, const std::set<char> &alphabet,
                              const size_t &q0) {
  commands.push_back(
      tu4::Tu4SetLetter<size_t>(q0, letterToCheck_, letterToCheck_, q0 + 1));
  size_t curQ = q0 + 1;
  for (auto &child : childs_) {
    auto subCommands = child->to4(alphabet, curQ);
    for (auto &subCommand : subCommands)
      commands.push_back(subCommand);
  }

  for (auto &letter : alphabet)
    commands.push_back(tu4::Tu4SetLetter<size_t>(curQ, letter, letter, q0));

  return curQ;
}
