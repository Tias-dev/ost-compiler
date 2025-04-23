#ifndef TU4_RUNNER_HPP_
#define TU4_RUNNER_HPP_

#include "Compiler.hpp"
#include "Line.hpp"
#include "Tu4Command.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstddef>
#include <map>
#include <vector>
namespace tu4run {
template <typename TQ = size_t, typename CharT = char> class Tu4Runner {
private:
  Line<CharT> &line_;
  std::vector<std::map<CharT, tu4::tu4_union<TQ, CharT>>> commands_;
  TQ q_ = 0;

public:
  Tu4Runner(Line<CharT> &line, const compiler::Commands<TQ, CharT> &commands)
      : line_(line) {
		size_t maxQ = 0;
		for(auto& command : commands) 
			maxQ = std::max(command.q(), maxQ);
		
    commands_.assign(maxQ + 1, {});
    for (auto &command : commands) {
      commands_[command.q0()][command.letterToCheck()] = command;
		}
  }

  /**
   * @brief do command step
   *
   * @return true if executed command was terminating and false -- otherwise
   */
  bool step() {
    CharT c = line_.getLetter();
    if (!commands_[q_].contains(c))
      throw error::Tu4RunError<CharT>(line_, std::string(strfast() << "No command for state: [" << q_
                                         << "] and letter: [" << c << "]"));

    const tu4::tu4_union<TQ, CharT> &command = commands_[q_][c];
    if (command.isShift()) {
      tu4::MoveDirection dir = command.getMoveDirection();
			// std::cout << "Shifting, " << int(dir) << std::endl;
      switch (dir) {
      case tu4::MoveDirection::LEFT:
        line_.shiftLeft();
				break;
      case tu4::MoveDirection::RIGHT:
        line_.shiftRight();
				break;
      }
    } else if (command.isSetLetter()) {
      CharT letterToSet = command.getLetterToSet();
			// std::cout << "Setting letter, " << letterToSet << std::endl;
      line_.setLetter(letterToSet);
    } else {
      throw error::Tu4RunError<CharT>(line_, std::string(strfast() << "Given command: [" << command
                                       << "] no shift and no set letter"));
    }

		// std::cout << q_ << ":";
    q_ = command.q();
		// std::cout << q_ << std::endl;
		// std::cout << line_.line() << std::endl;
		// size_t cursor = line_.cursor();
		// for(size_t i = 0; i < cursor; ++i) 
		// 	std::cout << ' ';
		// std::cout << '^' << std::endl;
    return command.isTerm();
  }

	/**
	 * @brief Start executing commands in loop
	 *
	 * @return Executing steps count
	 */
  size_t loop() {
		size_t steps = 0;
		bool isTerm = false;
    while (!isTerm) {
			isTerm = step();
			++steps;
    }
		
		return steps;
  }
};
} // namespace tu4run
#endif // !TU4_RUNNER_HPP_
