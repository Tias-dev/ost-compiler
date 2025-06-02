#include "FilePosition.hpp"
#include "CharStream.hpp"
#include <fstream>

FilePosition::FilePosition(std::shared_ptr<std::string> fileName, size_t index)
: fileName_(fileName), index_(index) {
	std::ifstream file(*fileName);
	CharStream stream(file);

	char c;
	while(stream.position() < index) {
		stream >> c;
	}

	row_ = stream.row();
	column_ = stream.column();
}
