#ifndef LINE_HPP_
#define LINE_HPP_

#include "exception.hpp"
#include <cctype>
#include <string>
#include <string_view>
namespace tu4run {
template <typename CharT = char>
class Line {
	std::string data_;
	SIZE_T cursor_;
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

	SIZE_T cursor() const {
		return cursor_;
	}

	CharT getLetter() const {
		return data_[cursor_];
	}

	bool operator==(const Line<CharT> & other) const {
		auto & line1 = line(), &line2 = other.line();
		SIZE_T minLen = std::min(line1.size(), line2.size());
		for(SIZE_T i = 0; i < minLen; ++i) 
			if(line1[i] != line2[i]) 
				return false;
		for(SIZE_T i = minLen; i < line1.size(); ++i) 
			if(line1[i] != lambda_) 
				return false;	
		for(SIZE_T i = minLen; i < line2.size(); ++i) 
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
	auto strline = line.line();
	auto begin = std::begin(strline);
	auto end = std::prev(std::end(strline));
	SIZE_T cursorPos = strline.size() - 1;
	while(end != begin && *end == '_') {
		--end;
		--cursorPos;
	}
	++end;
	++cursorPos;
	while(cursorPos <= line.cursor())
		++end, ++cursorPos;

	os << std::string_view(begin, end) << "_\n";
	SIZE_T cursor = line.cursor();
	for(SIZE_T _ = 0; _ < cursor; ++_) 
		os << space;
	os << cursorMark;

	return os;
}
#endif // !LINE_HPP_
