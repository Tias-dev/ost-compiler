#ifndef TU4_RUNNER_HPP_
#define TU4_RUNNER_HPP_

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
    bool isTerm = false;
    if (breakpointManager_)
      breakpointManager_->process(nextCommand(), isTerm);

    while (!isTerm) {
      isTerm = step();
      ++steps;
      if (breakpointManager_)
        breakpointManager_->process(nextCommand(), isTerm);
    }

    return steps;
  }
};

Tu4Runner<size_t, char> *initRunner(const std::string &fileName,
                                    const std::string &line);

struct Tu4RunnerBreakpoints {
	std::shared_ptr<const std::set<size_t>> stateBreakpoints;
	std::shared_ptr<const std::set<size_t>> lineBreakpoints;
};

std::tuple<Tu4Runner<size_t, char>*, Tu4RunnerBreakpoints> initRunnerWithBreakpoints(const std::string &fileName,
                                    const std::string &line);
} // namespace tu4run
#endif // !TU4_RUNNER_HPP_
