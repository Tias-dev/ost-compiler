#include "CharStream.hpp"
#include <iterator>

char CharStream::getchar() {
  char result;
  if (buffer_.empty()) {
    is_ >> result;
  } else {
    auto last = --std::end(buffer_);
    result = *last;
    buffer_.erase(last);
  }

  return result;
}

void CharStream::ungetchar(char c) {
  buffer_.push_back(c);
}

void CharStream::ungetstr(const std::string & s) {
  for(char c : s) {
    buffer_.push_back(c);
  }
}

CharStream & CharStream::operator>>(char &c) {
  c = getchar();
  return *this;
}

CharStream & CharStream::operator<<(char c) {
  ungetchar(c);
  return *this;
}

CharStream & CharStream::operator<<(const std::string & s) {
  ungetstr(s);
  return *this;
}

