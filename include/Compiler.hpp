#ifndef COMPILER_HPP_
#define COMPILER_HPP_

#include "Bimap.hpp"
#include "Tu4Command.hpp"
#include <cstdio>
#include <list>
#include <set>
#include <unordered_map>
#include <vector>
void compileAndSaveProgram(const std::string &fileName, const std::string &libDir,
                    const std::string &outDir, bool useBinaryFormat,
                    bool enableBreakpoints, bool printDebugInfo = true);
namespace compiler {
template <typename TLetter> class Alphabet : public std::set<TLetter> {
  using parent = std::set<TLetter>;
  TLetter lambda_;

public:
  Alphabet(const TLetter lambda = '_', bool insertLambda = true)
      : lambda_(lambda) {
    if (insertLambda)
      this->insert(lambda);
  };

  Alphabet(const std::set<TLetter> &letters) {
    this->insert(std::begin(letters), std::end(letters));
  }

  Alphabet(const Alphabet<TLetter> &al1, const Alphabet<TLetter> &al2) {
    this->insert(std::begin(al1), std::end(al1));
    this->insert(std::begin(al2), std::end(al2));
  }

  Alphabet<TLetter> operator||(const Alphabet<TLetter> &other) const {
    Alphabet<TLetter> res(*this);
    res.insert(std::begin(other), std::end(other));

    return res;
  }

  Alphabet<char> operator/(const Alphabet<TLetter> &other) const {
    Alphabet<TLetter> res{this->lambda_, false};
    for (auto &letter : *this) {
      if (other.contains(letter))
        continue;
      res.insert(letter);
    }
    return res;
  }
};

template <typename TQ, typename TLetter = char>
class Commands : public std::list<tu4::tu4_union<TQ, TLetter>> {
	std::unordered_map<TQ, TQ> overridedEndStates_;
public:
  void shift(TQ shiftSize) {
    for (auto &command : *this)
      command.shift(shiftSize);
  }

  void shiftTo(TQ state) {
    TQ minQ = this->minQ();
    if (minQ > state) {
      TQ delta = minQ - state;
      for (auto &command : *this)
        command.shiftDown(delta);
    } else {
      TQ delta = state - minQ;
      for (auto &command : *this)
        command.shift(delta);
    }
  }

  TQ minQ() const {
    if (this->empty())
      return 0;
    TQ minQ = std::begin(*this)->q();
    for (auto &command : *this)
      if (command.q0() < minQ)
        minQ = command.q0();

    return minQ;
  }

  TQ deltaQ() const {
    if (this->empty())
      return 0;

    TQ maxQ = std::begin(*this)->q(), minQ = maxQ;
    for (auto &command : *this)
      if (command.q() > maxQ)
        maxQ = command.q();
      else if (command.q0() < minQ)
        minQ = command.q0();

    return maxQ - minQ;
  }

  void extend(const Commands<TQ, TLetter> &other) {
    for (auto &command : other)
      this->push_back(command);
  }

	// Add override request to cache
	// Need to call executeOverrides() to do apply override requests
	//
	// @param oldState end state that will be overrided
	// @param newState end state to override oldState
	void overrideEndState(TQ oldState, TQ newState) {
		if(overridedEndStates_.contains(oldState))
			throw std::logic_error("Trying to override end state but it is already overrided!");
		overridedEndStates_[oldState] = newState;
	}

	// After applying overrides all override requests clering
	void executeOverrides() {
		for(auto &command : *this)
			if(overridedEndStates_.contains(command.q()))
				command.updateEndState(overridedEndStates_[command.q()]);

		overridedEndStates_.clear();
	}
};

// ------------------------
// |  base commands type  |
// ------------------------
using commands_type = Commands<SIZE_T, char>;
using mt_name_t = std::string;
std::pair<commands_type, mt_name_t> compileProgram(const std::string &fileName, const std::string &libDir, bool useBinaryFormat,
                    bool enableBreakpoints, bool printDebugInfo = true);
namespace serializer {
void serialize(const commands_type &commands,
               const bimap<std::string, SIZE_T> &fileCodes,
               const std::string &outFileName, bool useBinaryFormat);
commands_type deserialize(const std::string &fileName, bool useBinaryFormat);
} // namespace serializer
} // namespace compiler
#endif // !COMPILER_HPP_
