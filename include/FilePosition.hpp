#ifndef FILE_POSITION_HPP_
#define FILE_POSITION_HPP_

#include "utils.hpp"
#include <memory>
class FilePosition {
	protected:
	std::shared_ptr<std::string> fileName_;
	size_t row_;
	size_t column_;
	size_t index_;
public:
	FilePosition( std::shared_ptr<std::string> fileName, size_t row, size_t column, size_t index)
		: fileName_(fileName), row_(row), column_(column), index_(index) {};

	FilePosition(std::shared_ptr<std::string> fileName, size_t index);

	std::string to_string() const {
		return strfast() << *fileName_ << ":" << row_ << ":" << column_;
	}

	const std::string & fileName() const {
		return *fileName_;
	}

	size_t row() const {
		return row_;
	}

	size_t column() const {
		return column_;
	}

	size_t index() const {
		return index_;
	}
};

using filepos_t = FilePosition;

#endif // !FILE_POSITION_HPP_
