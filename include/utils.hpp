#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
namespace log {
	class LogBase : public std::stringstream {
		std::ostream & os_;
		std::string prefix_;
		protected:
			LogBase(std::ostream & os, std::string prefix) : os_(os), prefix_(prefix) {}
		public:

		~LogBase() {
			os_ << "[" << prefix_ << "]: " << str() << std::endl;
		}
	};

	class log : public LogBase {
		public:
			log(std::ostream & os = std::cout) : LogBase(os, "LOG") {}
	};

	class warn : public LogBase {
		public:
			warn(std::ostream & os = std::cout) : LogBase(os, "WARN") {}
	};

	class error : public LogBase {
		public:
			error(std::ostream & os = std::cout) : LogBase(os, "ERROR") {}
	};
} // namespace log

template <typename A, typename B>
class bimap {
	std::map<A, B> forward_;
	std::map<B, A> backward_;
public:
	B & operator[](const A &a) {
		return forward_[a];
	}

	A& operator[](const B & b) {
		return backward_[b];
	}

	const B & operator[](const A &a) const {
		return forward_[a];
	}

	const A& operator[](const B & b) const {
		return backward_[b];
	}

	void add(const A& a, const B & b) {
		forward_[a] = b;
		backward_[b] = a;
	}

	bool contains(const A &a) {
		return forward_.contains(a);
	}
	bool contains(const B &b) {
		return backward_.contains(b);
	}
};

struct strfast : public std::stringstream {
public:
	std::string bump() {
		return str();
	}
};

#endif // !UTILS_HPP_
