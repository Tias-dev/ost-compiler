#ifndef BREAKPOINTER_HPP_
#define BREAKPOINTER_HPP_

#include "FilePosition.hpp"
#include <memory>
#include <stack>
#include <string>
class IBreakpointer {
public:
  virtual void onEnter(const FilePosition &beginPos,
                       const FilePosition &endPos) = 0;
  virtual void onExit() = 0;
  virtual std::string getCurrentPosition() = 0;
};

class FileBreakpointer : public IBreakpointer {
public:
  struct State {
		FilePosition begin, end;

    static std::string dump(const State &state);
    static State load(const std::string &s);
  };

private:
  std::string fileName_;
  std::shared_ptr<std::string> fileNamePtr_;
  std::stack<State> states_;

public:
  FileBreakpointer(std::string fileName)
      : fileName_(fileName),
        fileNamePtr_(std::make_shared<std::string>(fileName)) {}

  void onEnter(const FilePosition &beginPos,
               const FilePosition &endPos) override;
  void onExit() override;
  std::string getCurrentPosition() override;
};

class DummyBreakpointer : public IBreakpointer {
public:
  void onExit() override {}
  void onEnter(const FilePosition & beginPos, const FilePosition & endPos) override {}
  std::string getCurrentPosition() override { return ""; }
};

#endif // !BREAKPOINTER_HPP_
