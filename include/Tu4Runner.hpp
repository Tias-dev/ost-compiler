#ifndef TU4_RUNNER_HPP_
#define TU4_RUNNER_HPP_

#include "BreakpointManager.hpp"
#include "Compiler.hpp"
#include "DIContainer.hpp"
#include "Line.hpp"
#include "Tu4Command.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <vector>
namespace tu4run {
template <typename TQ = size_t, typename CharT = char> class Tu4Runner {
private:
  Line<CharT> line_;
  std::vector<std::map<CharT, tu4::tu4_union<TQ, CharT>>> commands_;
  TQ q_ = 0;
  bool terminated_ = false;

  std::unique_ptr<deps::DI<tu4::tu4_union<TQ>, bool &>> breakpointManager_ = {
      nullptr};

public:
  Tu4Runner(const Line<CharT> &line,
            const compiler::Commands<TQ, CharT> &commands)
      : line_(line) {
    size_t maxQ = 0;
    for (auto &command : commands)
      maxQ = std::max(command.q(), maxQ);

    commands_.assign(maxQ + 1, {});
    for (auto &command : commands) {
      commands_[command.q0()][command.letterToCheck()] = command;
    }
  }

	void reset(const Line<CharT> &newLine) {
		q_ = 0;
		line_ = newLine;
	}

  TQ q() const { return q_; }

  void addBreakpointManager(
      std::unique_ptr<deps::DI<tu4::tu4_union<TQ>, bool&>> &&manager) {
    if (breakpointManager_)
      breakpointManager_->inject(std::move(manager));
    else
      breakpointManager_ = std::move(manager);
  }

  const tu4::tu4_union<TQ, CharT> &nextCommand() const {
    char curLetter = line_.getLetter();
    if (!commands_[q_].contains(curLetter))
      throw error::Tu4RunError<CharT>(
          line_,
          std::string(strfast() << "No command for state: [" << q_
                                << "] and letter: [" << curLetter << "]"));
    return commands_[q_].at(curLetter);
  }
  const Line<CharT> &line() const { return line_; }
  bool terminated() const { return terminated_; }

  /**
   * @brief do command step
   *
   * @return true if executed command was terminating and false -- otherwise
   */
  bool step() {
    const tu4::tu4_union<TQ, CharT> &command = nextCommand();
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
      throw error::Tu4RunError<CharT>(
          line_, std::string(strfast() << "Given command: [" << command
                                       << "] no shift and no set letter"));
    }

    q_ = command.q();
    return terminated_ = command.isTerm();
  }

  /**
   * @brief Start executing commands in loop
   *
   * @return Executing steps count
   */
  size_t loop() {
    size_t steps = 0;
    bool stopSteps = false;
    if (breakpointManager_)
      breakpointManager_->process(nextCommand(), stopSteps);

    while (!stopSteps) {
      step();
			stopSteps = terminated();
      ++steps;
      if (breakpointManager_)
        breakpointManager_->process(nextCommand(), stopSteps);
    }

    return steps;
  }
};

std::unique_ptr<Tu4Runner<size_t, char>> initRunner(const std::string &fileName,
                                    const std::string &line);

struct Tu4RunnerBreakpoints {
	std::shared_ptr<StateBreakpointManager<size_t>::breakpoints_t> stateBreakpoints;
	std::shared_ptr<LineBreakpointManager<size_t>::breakpoints_t> lineBreakpoints;
};


struct RunnerDataWithBreakpoints {
	std::unique_ptr<tu4run::Tu4Runner<size_t, char>> runner;
	tu4run::Tu4RunnerBreakpoints breakpoints;
	std::vector<std::string> fileNames;
};

RunnerDataWithBreakpoints initRunnerWithBreakpoints(const std::string &fileName,
                                    const std::string &line);
} // namespace tu4run
#endif // !TU4_RUNNER_HPP_
