#ifndef LINE_HPP_
#define LINE_HPP_

#include "exception.hpp"
#include <cctype>
#include <string>
namespace tu4run {
template <typename CharT = char>
class Line {
	std::string data_;
	size_t cursor_;
	CharT lambda_;
public:
	Line(const std::string & data, CharT lambda =  CharT('_'))
		: data_(data), lambda_(lambda) {
		for(auto& c : data_) 
			if(isspace(c)) 
				c = lambda_;
		
		data_ += lambda_;
		if(data_[0] != lambda_) 
			data_.insert(std::begin(data_), lambda_);
			
		if(data_.size() == 1) {
			cursor_ = 0;
			return;
		}

		cursor_ = data_.size() - 1;
		while(cursor_ > 0 && data_[cursor_] == lambda_) {
			--cursor_;
		}
		if(data_[cursor_] != lambda_) 
			++cursor_;
	}

	void shiftLeft() {
		if(cursor_ > 0) 
			--cursor_;
		else
		 throw error::Tu4RunError<CharT>(*this, "Trying to go out of line from left border");
	}

	void shiftRight() {
		if(cursor_ + 1 == data_.size()) 
			data_.push_back(lambda_);
		++cursor_;
	}

	void setLetter(CharT letter) {
		data_[cursor_] = letter;
	}

	const std::basic_string<CharT> & line() const {
		return data_;
	}

	size_t cursor() const {
		return cursor_;
	}

	CharT getLetter() const {
		return data_[cursor_];
	}

	bool operator==(const Line<CharT> & other) const {
		auto & line1 = line(), &line2 = other.line();
		size_t minLen = std::min(line1.size(), line2.size());
		for(size_t i = 0; i < minLen; ++i) 
			if(line1[i] != line2[i]) 
				return false;
		for(size_t i = minLen; i < line1.size(); ++i) 
			if(line1[i] != lambda_) 
				return false;	
		for(size_t i = minLen; i < line2.size(); ++i) 
			if(line2[i] != other.lambda_) 
				return false;	
			
		return cursor() == other.cursor();
	}

	bool operator!=(const Line<CharT> & other) const {
		return !(*this == other);
	}
};

} // namespace tu4run

template <typename CharT>
std::ostream & operator<<(std::ostream & os, const tu4run::Line<CharT> & line) {
	static const std::string space{' '}, cursorMark{"^\n"};
	os << line.line() << '\n';
	size_t cursor = line.cursor();
	for(size_t _ = 0; _ < cursor; ++_) 
		os << space;
	os << cursorMark;

	return os;
}
#endif // !LINE_HPP_
