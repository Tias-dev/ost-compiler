#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <sstream>
template <typename A, typename B> class bimap {
  std::map<A, B> forward_;
  std::map<B, A> backward_;

public:
  bimap() = default;
  bimap(std::list<std::pair<A, B>> base) {
    for (auto &[a, b] : base)
      this->add(a, b);
  }
  B &operator[](const A &a) { return forward_[a]; }

  A &operator[](const B &b) { return backward_[b]; }

  const B &operator[](const A &a) const { return forward_[a]; }

  const A &operator[](const B &b) const { return backward_[b]; }

  void add(const A &a, const B &b) {
    forward_[a] = b;
    backward_[b] = a;
  }

  bool contains(const A &a) { return forward_.contains(a); }
  bool contains(const B &b) { return backward_.contains(b); }
	size_t size() {return forward_.size();}

	void print(std::ostream & os = std::cout) {
		os << "-----------------"<< std::endl;
		for(auto& [k, v] : forward_) 
			os << k << ": " << v << std::endl;

		os << "~~~~~~~~~~~~~~~~~" << std::endl;
		for(auto& [k, v] : backward_) 
			os << k << ": " << v << std::endl;
		os << "-----------------"<< std::endl;
		
	}
};

struct strfast : public std::stringstream {
public:
  std::string bump() { return str(); }
	operator std::string() {return bump();}
};

template <class ...TS>
struct overloads : TS... {
	using TS::operator()...;
};

void fileRollAround(const std::string & fileName, size_t position, size_t width);
#endif // !UTILS_HPP_
