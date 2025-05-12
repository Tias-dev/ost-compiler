#ifndef BREAKPOINTER_HPP_
#define BREAKPOINTER_HPP_

#include <stack>
#include <string>
class IBreakpointer {
public:
  virtual void onEnter(size_t beginPos, size_t endPos) = 0;
  virtual void onExit() = 0;
  virtual std::string getCurrentPosition() = 0;
};

class FileBreakpointer : public IBreakpointer {
  std::string fileName_;
  std::stack<std::pair<size_t, size_t>> states_;

public:
  FileBreakpointer(std::string fileName) : fileName_(fileName) {}

  struct State {
    std::string fileName;
    size_t begin;
    size_t end;

    static std::string dump(const State &state);
    static State load(const std::string &s);
  };

  void onEnter(size_t beginPos, size_t endPos) override;
  void onExit() override;
  std::string getCurrentPosition() override;
};

class DummyBreakpointer : public IBreakpointer {
public:
  void onExit() override {}
  void onEnter(size_t beginPos, size_t endPos) override {}
  std::string getCurrentPosition() override { return ""; }
};

#endif // !BREAKPOINTER_HPP_
