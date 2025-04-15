#ifndef CHAR_STREAM_HPP_
#define CHAR_STREAM_HPP_

#include <cstddef>
#include <istream>
#include <queue>

class ICharStream {
	public:
		virtual size_t position() = 0;
		virtual ICharStream & operator>>(char &) = 0;
		virtual ICharStream & operator<<(char) = 0;
		virtual bool eof() = 0;
};

class CharStream : public ICharStream {
	std::queue<char> buffer_ = {};
	size_t position_ = 0;
	std::istream & is_;
	public:
	CharStream(std::istream & is) : is_(is) {}
	
	size_t position() override {return position_;}
	ICharStream & operator>>(char &c) override {
		++position_;
		if(!buffer_.empty()) {
			c = buffer_.front();
			buffer_.pop();
		} else {
			is_ >> c;
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
