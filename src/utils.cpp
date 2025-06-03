#include "utils.hpp"
#include "FilePosition.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <string>

void fileRollAround(const std::string & fileName, const FilePosition &position, size_t width) {
    std::fstream fileRunner(fileName);
		size_t row = 0, column = 1;
		std::string line;
		while(row < position.row()) {
			std::getline(fileRunner, line);
		}
		std::cout << line.substr(position.column() > width/2 ? position.column() - width / 2 : 0, width);
    std::cout << std::endl;
    for (size_t i = 0; i < position.column(); ++i)
      std::cout << " ";
    std::cout << "^" << std::endl;
}

std::string strip(const std::string & s) {
	std::string result;
	if(s.size() == 0) 
		return result;
		
	size_t i = 0, j = s.size()-1;
	while(i < s.size() && s[i] == ' ') 
		++i;
	
	while(j > 0 && s[j] == ' ') 
		--j;

	for(size_t k = i; k <= j; ++k) 
		result.push_back(s[k]);

	return result;
}

std::list<std::string> split(const std::string &s, char sep, bool includeEmpty) {
	std::list<std::string> result;
	std::string word;

	for(auto& c : s) 
		if(c == sep) {
			if(includeEmpty || word.size() > 0) 
				result.push_back(word);
			word = "";
		} else 
			word.push_back(c);

	if(includeEmpty || word.size() > 0) 
		result.push_back(word);

	return result;
}
