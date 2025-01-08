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

  onGetChar_.invoke(result);
  return result;
}

void CharStream::ungetchar(char c) {
  buffer_.push_back(c);
  onReturnChar_.invoke(c);
}

void CharStream::ungetstr(const std::string & s) {
  for(char c : s) {
    buffer_.push_back(c);
    onReturnChar_.invoke(c);
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

