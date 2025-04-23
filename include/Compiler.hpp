#ifndef COMPILER_HPP_
#define COMPILER_HPP_

#include "Tu4Command.hpp"
#include <list>
#include <set>
namespace compiler {
template <typename TLetter> class Alphabet : public std::set<TLetter> {
  using parent = std::set<TLetter>;
	TLetter lambda_;
public:
  Alphabet(const TLetter lambda = '_'): lambda_(lambda) {
		this->insert(lambda);
	};
	Alphabet(const std::set<TLetter> & letters) {
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

	Alphabet<char> operator/(const Alphabet<TLetter> & other) const {
		Alphabet<TLetter> res{this->lambda_};
		for(auto& letter : *this) {
			if(other.contains(letter)) 
				continue;
			res.insert(letter);
		}
		return res;
	}

};

template <typename TQ, typename TLetter = char>
class Commands : public std::list<tu4::tu4_union<TQ, TLetter>> {
public:
  void shift(TQ shiftSize) {
    for (auto &command : *this)
      command.shift(shiftSize);
  }
	
	void shiftTo(TQ state) {
		TQ minQ = this->minQ();
		if(minQ > state) {
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
		if(this->empty()) 
			return 0;
    TQ minQ =  std::begin(*this)->q();
    for (auto &command : *this)
      if(command.q0() < minQ) 
				minQ = command.q0();

		return minQ;
	}

  TQ deltaQ() const {
		if(this->empty()) 
			return 0;

    TQ maxQ = std::begin(*this)->q(), minQ = maxQ;
    for (auto &command : *this)
      if (command.q() > maxQ) 
        maxQ = command.q();
			 else if(command.q0() < minQ) 
				minQ = command.q0();

		return maxQ - minQ;
  }

	void extend(const Commands<TQ, TLetter> & other) {
		for(auto& command : other) 
			this->push_back(command);
	}
};

// ------------------------
// |  base commands type  |
// ------------------------
using commands_type = Commands<size_t, char>;
} // namespace compiler
#endif // !COMPILER_HPP_
