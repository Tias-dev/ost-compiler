#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
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

#endif // !UTILS_HPP_
