#ifndef FILE_POSITION_HPP_
#define FILE_POSITION_HPP_

#include <cassert>
#include <memory>
#include <ostream>
#include <vector>

#include "Bimap.hpp"

class IFileRoller {
 public:
  struct Position {
    SIZE_T row;
    SIZE_T column;
  };

  virtual Position convert(SIZE_T index) const = 0;
  virtual std::shared_ptr<std::string> fileName() const = 0;
};
class FileRoller : public IFileRoller {
  std::shared_ptr<std::string> fileName_;
  std::vector<SIZE_T> newLines_;

 public:
  FileRoller(std::shared_ptr<std::string> fileName);

  Position convert(SIZE_T index) const override;
  std::shared_ptr<std::string> fileName() const override { return fileName_; }
};

class FilePosition {
 protected:
  SIZE_T code_;
  SIZE_T row_;
  SIZE_T column_;
  static bimap<std::string, SIZE_T> fileCodesBimap_;

 public:
  FilePosition() = default;
  FilePosition(const std::string& fileName) {
    if (fileCodesBimap_.contains(fileName)) {
      code_ = fileCodesBimap_[fileName];
    } else {
      code_ = fileCodesBimap_.size();
      fileCodesBimap_.add(fileName, code_);
    }
  }
  FilePosition(std::shared_ptr<std::string> fileName, SIZE_T row, SIZE_T column)
      : FilePosition(*fileName) {
    row_ = row, column_ = column;
  };

  FilePosition(const IFileRoller& roller, SIZE_T index)
      : FilePosition(*roller.fileName()) {
    auto pos = roller.convert(index);
    row_ = pos.row;
    column_ = pos.column;
  }

  std::string to_string() const;
  SIZE_T code() const { return code_; }
  static FilePosition from_string(const std::string& s);
  static bimap<std::string, SIZE_T>& fileCodesBimap();
  const std::string& fileName() const { return fileCodesBimap_[code_]; }

  SIZE_T row() const { return row_; }

  SIZE_T column() const { return column_; }

  bool operator<(const FilePosition& other) const {
    if (row_ == other.row_) return column_ < other.column_;
    return row_ < other.row_;
  }
};

// ---------------------------
// |  2 positions in 1 file  |
// ---------------------------
using row_t = SIZE_T;
using column_t = SIZE_T;
struct FileRange {
  SIZE_T fileCode;
  std::pair<row_t, column_t> begin, end;

  static FileRange fromPositions(const FilePosition& begin,
                                 const FilePosition& end) {
    assert(
        ("Positions must point on the same file", begin.code() == end.code()));

    return {.fileCode = begin.code(),
            .begin = {begin.row(), begin.column()},
            .end = {end.row(), end.column()}};
  }

  static FileRange fromString(const std::string& s);
};

inline std::ostream& operator<<(std::ostream& os, const FileRange& range) {
  os << range.fileCode << ',' << range.begin.first << ',' << range.begin.second
     << ';' << range.end.first << ',' << range.end.second;
  return os;
}
#endif  // !FILE_POSITION_HPP_
