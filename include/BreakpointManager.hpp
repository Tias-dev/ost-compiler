#ifndef BREAKPOINT_MANAGER_HPP_
#define BREAKPOINT_MANAGER_HPP_

#include "Tu4Command.hpp"
#include "DIContainer.hpp"
#include <functional>
#include <memory>
#include <set>

namespace tu4run {
template <typename TQ>
class StateBreakpointManager : public deps::DI<tu4::tu4_union<TQ>, bool&> {
	using parent_t = deps::DI<tu4::tu4_union<TQ>>;
	std::shared_ptr<std::set<size_t>> states_ = {{}};
public:
	StateBreakpointManager()
		: parent_t([this](tu4::tu4_union<TQ> command, bool& isTerm) {
					if(this->breakpoints()->contains(command.q0())) 
						isTerm = true;
				}) {}
	std::shared_ptr<const std::set<TQ>> breakpoints() const {return states_;}

	void setBreakpoint(TQ q) {
		states_->insert(q);
	}

	void removeBreakpoints(TQ q) {
		if(states_->contains(q)) 
			states_->erase(q);
	}
};

template <typename TQ>
class LineBreakpointManager : public deps::DI<tu4::tu4_union<TQ>, bool&> {
	using parent_t = deps::DI<tu4::tu4_union<TQ>>;
	std::shared_ptr<std::set<size_t>> lines_ = {{}};
public:

	LineBreakpointManager()
		: parent_t([this](tu4::tu4_union<TQ> command, bool & isTerm) {
				auto state = FileBreakpointer::State::load(command.comment());
					if(this->breakpoints()->contains(state.begin.row())) 
					isTerm = true;
				}){}

	std::shared_ptr<const std::set<size_t>> breakpoints() const {return lines_;}

	void setBreakpoint(size_t line) {
		lines_->insert(line);
	}

	void removeBreakpoints(size_t line) {
		if(lines_->contains(line)) 
			lines_->erase(line);
	}
};
} // namespace tu4run
#endif // !BREAKPOINT_MANAGER_HPP_
