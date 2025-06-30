#ifndef DI_CONTAINER_HPP_
#define DI_CONTAINER_HPP_

#include <csignal>
#include <functional>
#include <memory>
#include <utility>
namespace deps {

template <typename ...Args>
class DI {
	std::unique_ptr<DI<Args...>> next_ = {nullptr};
protected:
	std::function<void(Args...)> handler_;
public:
	DI(std::function<void(Args...)> handler)
		: handler_(handler) {}

	DI & inject(std::unique_ptr<DI<Args...>> && dep) {
		dep->next_ = std::move(next_);
		next_ = std::move(dep);
		
		return *this;
	}

	DI & process(Args... args) {
		handler_(args...);
		if(next_) 
			next_->process(args...);

		return *this;
	}
};
} // namespace deps

#endif // !DI_CONTAINER_HPP_
