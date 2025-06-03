#ifndef FILE_POSITION_HPP_
#define FILE_POSITION_HPP_

#include <memory>
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
  std::shared_ptr<std::string> fileName_;
  size_t row_;
  size_t column_;
	const static std::shared_ptr<std::string> defaultFName_;
public:

  FilePosition(): fileName_(defaultFName_) {}
	FilePosition(std::shared_ptr<std::string> fileName, size_t row, size_t column)
      : fileName_(fileName), row_(row), column_(column) {};

  FilePosition(const IFileRoller &roller, size_t index)
      : fileName_(roller.fileName()) {
    auto pos = roller.convert(index);
    row_ = pos.row;
    column_ = pos.column;
  }

  std::string to_string() const;
	static FilePosition from_string(const std::string & s);
  const std::string &fileName() const { return *fileName_; }

  size_t row() const { return row_; }

  size_t column() const { return column_; }

	bool operator<(const FilePosition & other) const {
		if(row_ == other.row_)
			return column_ < other.column_;
		return row_ < other.row_;
	}
};

using filepos_t = FilePosition;

#endif // !FILE_POSITION_HPP_
