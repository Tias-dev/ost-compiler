#ifndef BREAKPOINTER_HPP_
#define BREAKPOINTER_HPP_

#include "FilePosition.hpp"
#include <stack>
#include <string>
class IBreakpointer {
public:
  virtual void onEnter(const FileRange &) = 0;
  virtual void onExit() = 0;
  virtual FileRange getCurrentPosition() = 0;
};

class FileBreakpointer : public IBreakpointer {
public:
  struct State {
		FileRange range;

    static std::string dump(const State &state);
    static State load(const std::string &s);
  };

private:
  std::stack<State> states_;
	State lastState_;

public:
  FileBreakpointer() = default;

  void onEnter(const FileRange &) override;
  void onExit() override;
  FileRange getCurrentPosition() override;
};

class DummyBreakpointer : public IBreakpointer {
public:
  void onExit() override {}
  void onEnter(const FileRange &) override {}
  FileRange getCurrentPosition() override { return {}; }
};

#endif // !BREAKPOINTER_HPP_
