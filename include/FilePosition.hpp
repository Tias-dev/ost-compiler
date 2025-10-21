#ifndef FILE_POSITION_HPP_
#define FILE_POSITION_HPP_

#include "Bimap.hpp"
#include <cassert>
#include <memory>
#include <ostream>
#include <vector>

class IFileRoller {
public:
  struct Position {
    size_t row;
    size_t column;
  };

  virtual Position convert(size_t index) const = 0;
  virtual std::shared_ptr<std::string> fileName() const = 0;
};
class FileRoller : public IFileRoller {
  std::shared_ptr<std::string> fileName_;
  std::vector<size_t> newLines_;

public:
  FileRoller(std::shared_ptr<std::string> fileName);

  Position convert(size_t index) const override;
  std::shared_ptr<std::string> fileName() const override { return fileName_; }
};

class FilePosition {
protected:
  size_t code_;
  size_t row_;
  size_t column_;
  static bimap<std::string, size_t> fileCodesBimap_;

public:
	FilePosition() = default;
  FilePosition(const std::string &fileName) {
    if (fileCodesBimap_.contains(fileName)) {
      code_ = fileCodesBimap_[fileName];
    } else {
      code_ = fileCodesBimap_.size();
			fileCodesBimap_.add(fileName, code_);
    }
  }
  FilePosition(std::shared_ptr<std::string> fileName, size_t row, size_t column)
      : FilePosition(*fileName) {
    row_ = row, column_ = column;
  };

  FilePosition(const IFileRoller &roller, size_t index)
      : FilePosition(*roller.fileName()) {
    auto pos = roller.convert(index);
    row_ = pos.row;
    column_ = pos.column;
  }

  std::string to_string() const;
  size_t code() const { return code_; }
  static FilePosition from_string(const std::string &s);
  static bimap<std::string, size_t> &fileCodesBimap();
	const std::string &fileName() const { return fileCodesBimap_[code_]; }

  size_t row() const { return row_; }

  size_t column() const { return column_; }

  bool operator<(const FilePosition &other) const {
    if (row_ == other.row_)
      return column_ < other.column_;
    return row_ < other.row_;
  }
};

// ---------------------------
// |  2 positions in 1 file  |
// ---------------------------
using row_t = size_t;
using column_t = size_t;
struct FileRange {
	size_t fileCode;
	std::pair<row_t, column_t> begin, end;

	static FileRange fromPositions(const FilePosition &begin, const FilePosition & end) {
		assert(("Positions must point on the same file", begin.code() == end.code()));

		return {.fileCode = begin.code(), .begin = {begin.row(), begin.column()}, .end = {end.row(), end.column()}};
	}

	static FileRange fromString(const std::string &s);
};

inline std::ostream & operator<<(std::ostream & os, const FileRange & range) {
	os << range.fileCode << ',' << range.begin.first << ',' << range.begin.second << ';' << range.end.first << ',' << range.end.second;
	return os;
}
#endif // !FILE_POSITION_HPP_
