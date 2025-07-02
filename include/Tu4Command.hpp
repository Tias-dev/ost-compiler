#ifndef TU_4_COMMAND_HPP_
#define TU_4_COMMAND_HPP_

#include "exception.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
namespace tu4 {
enum class MoveDirection { LEFT, RIGHT };

template <typename TQ, typename TLetter = char> class Tu4Command {
  TQ q0_, q_;
  TLetter letterToCheck_;
  std::string comment_;

public:
  Tu4Command(TQ q0, TQ q, TLetter letterToCheck, const std::string &comment)
      : q0_(q0), q_(q), letterToCheck_(letterToCheck), comment_(comment) {
    if (globals::enableBreakpoints && comment == "")
      comment_ = globals::breakpointer->getCurrentPosition();
  }

  TQ q0() const { return q0_; }
  TQ q() const { return q_; }
  TLetter letterToCheck() const { return letterToCheck_; }
  const std::string &comment() const { return comment_; }

  void shift(TQ shiftSize) {
    q0_ += shiftSize;
    q_ += shiftSize;
  }

  void shiftDown(TQ shiftSize) {
    q0_ -= shiftSize;
    q_ -= shiftSize;
  }

  virtual bool isTerm() const = 0;
  virtual void print(std::ostream &os) const = 0;
};

template <typename TQ, typename TLetter = char>
class Tu4SetLetter : public Tu4Command<TQ, TLetter> {
  TLetter letterToSet_;

public:
  Tu4SetLetter(TQ q0, TLetter letterToCheck, TLetter letterToSet, TQ q,
               const std::string &comment = "")
      : Tu4Command<TQ>(q0, q, letterToCheck, comment),
        letterToSet_(letterToSet) {}

  void print(std::ostream &os) const override {
    os << this->q0() << ',' << this->letterToCheck() << ',' << letterToSet_
       << ',' << this->q();
    if (this->comment().size() > 0)
      os << " // " << this->comment();
  }
  bool isTerm() const override {
    return (this->letterToCheck() == letterToSet_) && (this->q0() == this->q());
  }

  TLetter letterToSet() const { return letterToSet_; }
};

template <typename TQ, typename TLetter = char>
class Tu4Move : public Tu4Command<TQ, TLetter> {
  MoveDirection dir_;

public:
  Tu4Move(TQ q0, TLetter letterToCheck, MoveDirection dir, TQ q,
          const std::string &comment = "")
      : Tu4Command<TQ>(q0, q, letterToCheck, comment), dir_(dir) {}

  void print(std::ostream &os) const override {
    os << this->q0() << ',' << this->letterToCheck() << ','
       << (dir_ == MoveDirection::LEFT ? "<" : ">") << ',' << this->q();
    if (this->comment().size() > 0)
      os << " // " << this->comment();
  }

  bool isTerm() const override { return false; }
  MoveDirection moveDirection() const { return dir_; }
};

template <typename TQ, typename TLetter = char> class tu4_union {
  using data_type =
      std::variant<Tu4SetLetter<TQ, TLetter>, Tu4Move<TQ, TLetter>>;
  std::optional<data_type> data_;

public:
  tu4_union() = default;
  tu4_union(data_type data) : data_(data) {}

  tu4_union(const tu4_union<TQ, TLetter> &other) : data_(other.data_) {}
  tu4_union(tu4_union<TQ, TLetter> &&other) : data_(std::move(other.data_)) {}

  tu4_union<TQ, TLetter> &operator=(const tu4_union<TQ, TLetter> &other) {
    data_ = other.data_;

    return *this;
  }

  tu4_union<TQ, TLetter> &operator=(tu4_union<TQ, TLetter> &&other) {
    data_ = std::move(other.data_);

    return *this;
  }

  void shift(TQ shiftSize) {
    std::visit([shiftSize](auto &command) { command.shift(shiftSize); },
               *data_);
  }

  TQ q0() const {
    return std::visit([](const auto &command) { return command.q0(); }, *data_);
  }
  TQ q() const {
    return std::visit([](const auto &command) { return command.q(); }, *data_);
  }

  TLetter letterToCheck() const {
    return std::visit(
        [](const auto &command) { return command.letterToCheck(); }, *data_);
  }

  std::string comment() const {
    return std::visit([](const auto &command) { return command.comment(); },
                      *data_);
  }

  void print(std::ostream &os) const {
    std::visit([&os](const auto &command) { command.print(os); }, *data_);
  }

  bool isTerm() const {
    return std::visit([](const auto &command) { return command.isTerm(); },
                      *data_);
  }

  bool isShift() const {
    return std::visit(overloads{[](const Tu4SetLetter<TQ> &) { return false; },
                                [](const Tu4Move<TQ> &) { return true; }},
                      *data_);
  }

  MoveDirection getMoveDirection() const {
    return std::visit(
        overloads{
            [](const Tu4SetLetter<TQ> &) {
              throw std::logic_error(
                  "Set letter command has no move direction");
              return MoveDirection::LEFT;
            },
            [](const Tu4Move<TQ> &command) { return command.moveDirection(); }},
        *data_);
  }

  bool isSetLetter() const {
    return std::visit(overloads{[](const Tu4SetLetter<TQ> &c) { return true; },
                                [](const Tu4Move<TQ> &) { return false; }},
                      *data_);
  }
  TLetter getLetterToSet() const {
    return std::visit(
        overloads{[](const Tu4SetLetter<TQ> &c) { return c.letterToSet(); },
                  [](const Tu4Move<TQ> &) {
                    throw std::logic_error("Move command has no letter to set");
                    return TLetter(0);
                  }},
        *data_);
  }

  void shiftDown(TQ shiftSize) {
    std::visit([shiftSize](auto &command) { command.shiftDown(shiftSize); },
               *data_);
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
	static auto isValidLine = [](const std::string &s) {
		if(s.empty())
			return false;
		size_t commandEnd = 1, commaCount = 0;
		for(commandEnd = 1; commandEnd < s.size(); ++commandEnd) {
			if(s[commandEnd == '/'] && s[commandEnd - 1] == '/') {
				--commandEnd;
				break;
			} else if(s[commandEnd] == ',') {
				++commaCount;
			}
		}
		if(commaCount != 3)
			return false;
		return true;
	};

  std::string line, comment = "";

  do {
		logger::debug() << "Skipped not valid line [" << line << ']';
    std::getline(is, line);
  } while (is && !(isValidLine(line)));
  if (!isValidLine(line))
    throw error::UnexpectedFileEnd();

  std::istringstream iss(line);
  TQ q0, q;
  TLetter c1, c2, temp;
  iss >> q0 >> temp >> c1 >> temp >> c2 >> temp >> q;
  size_t commentPos;

  if ((commentPos = line.find("//")) != std::string::npos) {
    while (commentPos + 2 < line.size() && line[commentPos + 2] == ' ')
      ++commentPos;
    comment = line.substr(commentPos + 2);
  }

  if (c1 == TLetter(' '))
    c1 = TLetter('_');
  if (c2 == TLetter(' '))
    c2 = TLetter('_');

  switch (c2) {
  case '>':
  case '<':
    return {tu4::Tu4Move<TQ, TLetter>{
        q0, c1,
        (c2 == '<' ? tu4::MoveDirection::LEFT : tu4::MoveDirection::RIGHT), q,
        comment}};
  default:
    return {tu4::Tu4SetLetter<TQ, TLetter>(q0, c1, c2, q, comment)};
  }
}

template <typename TQ, typename TLetter = char>
std::list<tu4::tu4_union<TQ, TLetter>> loadMultiple(std::istream &is) {
  std::list<tu4::tu4_union<TQ, TLetter>> commands;
  try {
    while (true) {
      commands.push_back(load<size_t, char>(is));
    }
  } catch (error::UnexpectedFileEnd) {
  }

  return commands;
}

#endif // !TU_4_COMMAND_HPP_
