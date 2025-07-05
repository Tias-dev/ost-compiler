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
		virtual size_t column() = 0;
		virtual size_t row() = 0;
};

class CharStream : public ICharStream {
	std::stack<char> buffer_ = {};
	std::stack<size_t> linesLen_ = {};
	size_t position_ = 0;
	size_t row_ = 1, column_ = 1;
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

		if(c == '\n') {
			++row_;
			linesLen_.push(column_);
			column_ = 1;
		} else
			++column_;
		return *this;
	}

	ICharStream & operator<<(char c) override {
		if(position_) 
			--position_;
		buffer_.push(c);
		if(c == '\n' && row_ > 0) {
			--row_;
			column_ = linesLen_.top();
			linesLen_.pop();
		}
		return *this;
	}

	bool eof() override {
		return buffer_.empty() && is_.eof();
	}
	size_t row() override {
		return row_;
	}
	size_t column() override {
		return column_;
	}
};

#endif // !CHAR_STREAM_HPP_
