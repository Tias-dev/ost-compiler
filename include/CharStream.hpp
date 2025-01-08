#ifndef CHAR_STREAM_HPP_
#define CHAR_STREAM_HPP_

#include "event.hpp"
#include <istream>
#include <list>
#include <stack>
#include <string>

class ICharStream {
public:
  ICharStream(IEvent<char> &onGetCh, IEvent<char> &onReturnCh)
      : onGetChar(onGetCh), onReturnChar(onReturnCh) {}
  virtual ~ICharStream() {}

  IEvent<char> &onGetChar;
  IEvent<char> &onReturnChar;

  virtual ICharStream &operator>>(char &c) = 0;
  virtual ICharStream &operator<<(char c) = 0;
  virtual ICharStream &operator<<(const std::string &s) = 0;
};

class CharStream : public ICharStream {
private:
  std::istream &is_;
  std::stack<char> buffer_;
  Event<char> onGetChar_;
  Event<char> onReturnChar_;

public:
  CharStream(std::istream &is, std::stack<char> buffer = {})
      : ICharStream(onGetChar_, onReturnChar_), is_(is), buffer_(buffer) {}

  char getchar();
  void ungetchar(char c);
  void ungetstr(const std::string &s);

  ICharStream &operator>>(char &c) override;
  ICharStream &operator<<(char c) override;
  ICharStream &operator<<(const std::string &s) override;
};

#endif // !CHAR_STREAM_HPP_
