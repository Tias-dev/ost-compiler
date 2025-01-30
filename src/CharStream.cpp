#include "CharStream.hpp"

char CharStream::getchar() {
  char result;
  if (buffer_.empty()) {
    result = is_.get();
  } else {
    char last = buffer_.top();
    result = last;
    buffer_.pop();
  }

  onGetChar_.invoke(result);
  return result;
}

void CharStream::ungetchar(char c) {
  buffer_.push(c);
  onReturnChar_.invoke(c);
}

void CharStream::ungetstr(const std::string & s) {
  for(auto it = --s.end(); it >= s.begin(); --it) {
    buffer_.push(*it);
    onReturnChar_.invoke(*it);
  }
}

ICharStream & CharStream::operator>>(char &c) {
  c = getchar();
  return *this;
}

ICharStream & CharStream::operator<<(char c) {
  ungetchar(c);
  return *this;
}

ICharStream & CharStream::operator<<(const std::string & s) {
  ungetstr(s);
  return *this;
}

