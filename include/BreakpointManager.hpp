#ifndef BREAKPOINT_MANAGER_HPP_
#define BREAKPOINT_MANAGER_HPP_

#include "DIContainer.hpp"
#include "Tu4Command.hpp"
#include <memory>
#include <set>

namespace tu4run {

template <typename TBreak> struct BreakpointStorage {
  std::set<TBreak> breakpoints;

  void add(TBreak b) { breakpoints.insert(b); }
  bool isBreakpoint(TBreak b) const { return breakpoints.contains(b); }
  void remove(TBreak b) {
    if (isBreakpoint(b))
      breakpoints.erase(b);
  }
};

template <typename TQ>
class StateBreakpointManager : public deps::DI<tu4::tu4_union<TQ>, bool &> {
  using parent_t = deps::DI<tu4::tu4_union<TQ>, bool&>;
  std::shared_ptr<BreakpointStorage<TQ>> states_ = std::make_shared<BreakpointStorage<TQ>>();

public:
  StateBreakpointManager()
      : parent_t([this](tu4::tu4_union<TQ> command, bool &isTerm) {
          if (this->breakpoints()->isBreakpoint(command.q0()))
            isTerm = true;
        }) {}
  std::shared_ptr<BreakpointStorage<TQ>> breakpoints() const { return states_; }

  void setBreakpoint(TQ q) { states_->add(q); }

  void removeBreakpoints(TQ q) {
		states_->remove(q);
  }
};

template <typename TQ>
class LineBreakpointManager : public deps::DI<tu4::tu4_union<TQ>, bool &> {
  using parent_t = deps::DI<tu4::tu4_union<TQ>, bool&>;
  std::shared_ptr<BreakpointStorage<size_t>> lines_ = std::make_shared<BreakpointStorage<size_t>>();

public:
  LineBreakpointManager()
      : parent_t([this](tu4::tu4_union<TQ> command, bool &isTerm) {
          auto state = FileBreakpointer::State::load(command.comment());
          if (this->breakpoints()->isBreakpoint(state.begin.row()))
            isTerm = true;
        }) {}

  std::shared_ptr<BreakpointStorage<size_t>> breakpoints() const { return lines_; }

  void setBreakpoint(size_t line) { lines_->add(line); }

  void removeBreakpoints(size_t line) {
		lines_->remove(line);
  }
};
} // namespace tu4run
#endif // !BREAKPOINT_MANAGER_HPP_
