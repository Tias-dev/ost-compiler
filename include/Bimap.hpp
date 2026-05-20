#ifndef BIMAP_HPP_
#define BIMAP_HPP_

#include <iostream>
#include <list>
#include <map>
template <typename A, typename B>
class bimap {
  std::map<A, B> forward_;
  std::map<B, A> backward_;

 public:
  bimap() = default;
  bimap(std::list<std::pair<A, B>> base) {
    for (auto& [a, b] : base) this->add(a, b);
  }
  B& operator[](const A& a) { return forward_[a]; }

  A& operator[](const B& b) { return backward_[b]; }

  const B& operator[](const A& a) const { return forward_[a]; }

  const A& operator[](const B& b) const { return backward_[b]; }

  const std::map<A, B>& forward() const { return forward_; }
  const std::map<B, A>& backward() const { return backward_; }

  void add(const A& a, const B& b) {
    forward_[a] = b;
    backward_[b] = a;
  }

  bool contains(const A& a) { return forward_.contains(a); }
  bool contains(const B& b) { return backward_.contains(b); }
  SIZE_T size() const { return forward_.size(); }
  void clear() {
    forward_.clear();
    backward_.clear();
  }

  void print(std::ostream& os = std::cout) {
    os << "-----------------" << '\n';
    for (auto& [k, v] : forward_) os << k << ": " << v << '\n';

    os << "~~~~~~~~~~~~~~~~~" << '\n';
    for (auto& [k, v] : backward_) os << k << ": " << v << '\n';
    os << "-----------------" << '\n';
  }
};
#endif  // !BIMAP_HPP_
