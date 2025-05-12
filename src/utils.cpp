#include "utils.hpp"
#include <fstream>
#include <iostream>


void fileRollAround(const std::string & fileName, size_t position, size_t width) {
    std::fstream fileRunner(fileName);
    for (size_t i = 0; i < position - std::min(position, width / 2); ++i)
      fileRunner.get();

    char c;
    for (size_t i = 0; i < width; ++i) {
      c = fileRunner.get();
      if (isspace(c))
        c = ' ';

      std::cout << c;
    }
    std::cout << std::endl;
    for (size_t i = 0; i < width / 2; ++i)
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
