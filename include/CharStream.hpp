#ifndef CHAR_STREAM_HPP_
#define CHAR_STREAM_HPP_

#include <cstddef>
#include <istream>
#include <queue>

class ICharStream {
	public:
		virtual size_t position() = 0;
		virtual ICharStream & operator>>(char &);
		virtual ICharStream & operator<<(char);
};

class CharStream : public ICharStream {
	std::queue<char> buffer = {};
	size_t position_ = 0;
	std::istream & is_;
	public:
	CharStream(std::istream & is) : is_(is) {}
	
	size_t position() override {return position_;}
	ICharStream & operator>>(char &c) override {
		++position_;
		if(!buffer.empty()) {
			c = buffer.front();
			buffer.pop();
		} else {
			is_ >> c;
		}
		return *this;
	}

	ICharStream & operator<<(char c) override {
		if(position_) 
			--position_;
		buffer.push(c);
		return *this;
	}
};

#endif // !CHAR_STREAM_HPP_
