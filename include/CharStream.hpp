#ifndef CHAR_STREAM_HPP_
#define CHAR_STREAM_HPP_

#include <istream>
#include <list>
#include <string>

class CharStream {
private:
  std::istream &is_;
  std::list<char> buffer_;

public:
  CharStream(std::istream &is, std::list<char> buffer = {})
      : is_(is), buffer_(buffer) {}

  char getchar(); 
  void ungetchar(char c);
  void ungetstr(const std::string & s);

  CharStream & operator>>(char &c);
  CharStream & operator<<(char c);
  CharStream & operator<<(const std::string & s);
};

#endif // !CHAR_STREAM_HPP_
