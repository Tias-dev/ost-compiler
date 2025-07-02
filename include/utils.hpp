#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "FilePosition.hpp"
#include "globals.hpp"
#include <cstddef>
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
  size_t size() { return forward_.size(); }

  void print(std::ostream &os = std::cout) {
    os << "-----------------" << std::endl;
    for (auto &[k, v] : forward_)
      os << k << ": " << v << std::endl;

    os << "~~~~~~~~~~~~~~~~~" << std::endl;
    for (auto &[k, v] : backward_)
      os << k << ": " << v << std::endl;
    os << "-----------------" << std::endl;
  }
};

struct strfast : public std::stringstream {
public:
  std::string bump() { return str(); }
  operator std::string() { return bump(); }
};

template <class... TS> struct overloads : TS... {
  using TS::operator()...;
};

void fileRollAround(const std::string &fileName, const FilePosition &position,
                    size_t width);

std::string strip(const std::string &s);
std::list<std::string> split(const std::string &s, char sep = ' ',
                             bool includeEmpty = false);

namespace logger {
enum class LogLevel : char {
  INFO = 'i',
  WARNING = 'w',
  ERROR = 'e',
  DEBUG = 'd'
};

struct log : public std::stringstream {
  LogLevel level_;
  bool canceled_ = false;
  log(LogLevel level = LogLevel::INFO) : level_(level) {}
  ~log() {
    if (canceled_)
      return;
    switch (level_) {
    case LogLevel::INFO:
      std::cout << "[INFO] " << str() << std::endl;
      break;
    case LogLevel::WARNING:
      std::cerr << "[WARNING] " << str() << std::endl;
      break;
    case LogLevel::ERROR:
      std::cerr << "[ERROR] " << str() << std::endl;
      break;
    case LogLevel::DEBUG:
      std::cout << "[DEBUG] " << str() << std::endl;
			break;
    default:
      std::cerr << "[WARNING] " << "Log level [" << char(level_)
                << "] not supported" << std::endl;
    }
  }
};

struct info : public log {
  info() : log(LogLevel::INFO) {}
};

struct warning : public log {
  warning() : log(LogLevel::WARNING) {}
};

struct error : public log {
  error() : log(LogLevel::ERROR) {}
};

struct debug : public log {
  debug() : log(LogLevel::DEBUG) { canceled_ = !globals::printDebugInfo; }
};
} // namespace logger
#endif // !UTILS_HPP_
