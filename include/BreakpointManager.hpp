#ifndef BREAKPOINT_MANAGER_HPP_
#define BREAKPOINT_MANAGER_HPP_

#include "DIContainer.hpp"
#include "FilePosition.hpp"
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
public:
  using breakpoints_t = BreakpointStorage<TQ>;

private:
  using parent_t = deps::DI<tu4::tu4_union<TQ>, bool &>;
  std::shared_ptr<breakpoints_t> states_ =
      std::make_shared<BreakpointStorage<TQ>>();

public:
  StateBreakpointManager()
      : parent_t([this](tu4::tu4_union<TQ> command, bool &isTerm) {
          if (this->breakpoints()->isBreakpoint(command.q0()))
            isTerm = true;
        }) {}
  std::shared_ptr<breakpoints_t> breakpoints() const { return states_; }

  void setBreakpoint(TQ q) { states_->add(q); }

  void removeBreakpoints(TQ q) { states_->remove(q); }
};

template <typename TQ>
class LineBreakpointManager : public deps::DI<tu4::tu4_union<TQ>, bool &> {
public:
  using breakpoints_t = std::map<size_t, BreakpointStorage<size_t>>;

private:
  using parent_t = deps::DI<tu4::tu4_union<TQ>, bool &>;
  std::shared_ptr<breakpoints_t> lines_ = std::make_shared<breakpoints_t>();

public:
  LineBreakpointManager()
      : parent_t([this](tu4::tu4_union<TQ> command, bool &isTerm) {
          FileRange range = command.debugBreakpoint().value();
          size_t row = range.begin.first;
          if (this->breakpoints()->contains(range.fileCode) &&
              (*this->breakpoints())[range.fileCode].isBreakpoint(row))
            isTerm = true;
        }) {}

  std::shared_ptr<breakpoints_t> breakpoints() const { return lines_; }

  void setBreakpoint(std::pair<const std::string &, size_t> line) {
    size_t fileCode = FilePosition::fileCodesBimap()[line.first];
    if (!lines_->contains(fileCode))
      (*lines_)[fileCode] = {};
    (*lines_)[fileCode].add(line.second);
  }

  void removeBreakpoints(std::pair<const std::string &, size_t> line) {
    size_t fileCode = FilePosition::fileCodesBimap()[line.first];
    if (lines_->contains(fileCode))
      (*lines_)[fileCode].remove(line.second);
  }
};
} // namespace tu4run
#endif // !BREAKPOINT_MANAGER_HPP_
