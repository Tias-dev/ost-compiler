#ifndef TU_4_COMMAND_HPP_
#define TU_4_COMMAND_HPP_

#include "exception.hpp"
#include <ostream>
#include <sstream>
#include <string>
#include <variant>
namespace tu4 {
enum class MoveDirection { LEFT, RIGHT };

template <typename TQ, typename TLetter = char> class Tu4Command {
  TQ q0_, q_;
  TLetter letterToCheck_;
  std::string comment_;

public:
  Tu4Command(TQ q0, TQ q, TLetter letterToCheck, std::string comment = "")
      : q0_(q0), q_(q), letterToCheck_(letterToCheck), comment_(comment) {}

  TQ q0() const { return q0_; }
  TQ q() const { return q_; }
  TLetter letterToCheck() const { return letterToCheck_; }
  const std::string &comment() const { return comment_; }

  void shift(TQ shiftSize) {
    q0_ += shiftSize;
    q_ += shiftSize;
  }

  virtual bool isTerm() const = 0;
  virtual void print(std::ostream &os) const = 0;
};

template <typename TQ, typename TLetter = char>
class Tu4SetLetter : public Tu4Command<TQ, TLetter> {
  TLetter letterToSet_;

public:
  Tu4SetLetter(TQ q0, TLetter letterToCheck, TLetter letterToSet, TQ q)
      : Tu4Command<TQ>(q0, q, letterToCheck), letterToSet_(letterToSet) {}

  void print(std::ostream &os) const override {
    os << this->q0() << ',' << this->letterToCheck() << ',' << letterToSet_
       << ',' << this->q();
    if (this->comment().size() > 0)
      os << " // " << this->comment();
  }
  bool isTerm() const override {
    return (this->letterToCheck() == letterToSet_) && (this->q0() == this->q());
  }
};

template <typename TQ, typename TLetter = char>
class Tu4Move : public Tu4Command<TQ, TLetter> {
  MoveDirection dir_;

public:
  Tu4Move(TQ q0, TLetter letterToCheck, MoveDirection dir, TQ q)
      : Tu4Command<TQ>(q0, q, letterToCheck), dir_(dir) {}

  void print(std::ostream &os) const override {
    os << this->q0() << ',' << this->letterToCheck() << ','
       << (dir_ == MoveDirection::LEFT ? "<" : ">") << ',' << this->q();
    if (this->comment().size() > 0)
      os << " // " << this->comment();
  }

  bool isTerm() const override { return false; }
};

template <typename TQ, typename TLetter = char> class tu4_union {
  using data_type =
      std::variant<Tu4SetLetter<TQ, TLetter>, Tu4Move<TQ, TLetter>>;
  data_type data_;

public:
  tu4_union(data_type data) : data_(data) {}

  void shift(TQ shiftSize) {
    std::visit([shiftSize](auto &command) { command.shift(shiftSize); }, data_);
  }

  TQ q0() const {
    return std::visit([](const auto &command) { return command.q0(); }, data_);
  }
  TQ q() const {
    return std::visit([](const auto &command) { return command.q(); }, data_);
  }

  TLetter letterToCheck() const {
    return std::visit(
        [](const auto &command) { return command.letterToCheck(); }, data_);
  }

  const std::string &comment() const {
    return std::visit([](const auto &command) { return command.comment(); },
                      data_);
  }

  void print(std::ostream &os) const {
    std::visit([&os](const auto &command) { command.print(os); }, data_);
  }

  bool isTerm() const {
    return std::visit([](const auto &command) { return command.isTerm(); }, data_);
  }
};
} // namespace tu4

template <typename TQ, typename TLetter = char>
std::ostream &operator<<(std::ostream &os,
                         const tu4::tu4_union<TQ, TLetter> &command) {
  command.print(os);
  return os;
}

template <typename TQ, typename TLetter = char>
tu4::tu4_union<TQ, TLetter> load(std::istream &is) {
  std::string line, comment = "";
  std::getline(is, line);
	if(line.size() < 2) 
		throw error::UnexpectedFileEnd();

  std::istringstream iss(line);
  TQ q0, q;
  TLetter c1, c2, temp;
  iss >> q0 >> temp >> c1 >> temp >> c2 >> temp >> q;
  if (line.find("//") != std::string::npos) {
    iss >> comment >> comment;
  }

  switch (c2) {
  case '>':
  case '<':
    return {tu4::Tu4Move<TQ, TLetter>{
        q0, c1,
        (c2 == '<' ? tu4::MoveDirection::LEFT : tu4::MoveDirection::RIGHT), q}};
  default:
    return {tu4::Tu4SetLetter<TQ, TLetter>(q0, c1, c2, q)};
  }
}

#endif // !TU_4_COMMAND_HPP_
