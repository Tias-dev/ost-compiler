#ifndef CHAR_STREAM_HPP_
#define CHAR_STREAM_HPP_

#include <cstddef>
#include <iostream>
#include <stack>

class ICharStream {
	public:
		virtual size_t position() = 0;
		virtual ICharStream & operator>>(char &) = 0;
		virtual ICharStream & operator<<(char) = 0;
		virtual bool eof() = 0;
};

class CharStream : public ICharStream {
	std::stack<char> buffer_ = {};
	size_t position_ = 0;
	std::istream & is_;
	public:
	CharStream(std::istream & is) : is_(is) {}
	
	size_t position() override {return position_;}
	ICharStream & operator>>(char &c) override {
		++position_;
		if(!buffer_.empty()) {
			c = buffer_.top();
			buffer_.pop();
		} else {
			c = is_.get();
		}
		return *this;
	}

	ICharStream & operator<<(char c) override {
		if(position_) 
			--position_;
		buffer_.push(c);
		return *this;
	}

	bool eof() override {
		return buffer_.empty() && is_.eof();
	}
};

#endif // !CHAR_STREAM_HPP_
