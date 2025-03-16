#include "CharStream.hpp"
#include "LexemAnalizer.hpp"
#include <cctype>
#include <fstream>
#include <iostream>
 
int main(int argc, char * argw[]) {
	std::fstream file("./testProgram.ost");
	if(!file.is_open()) {
		std::cout << "Can't open file" << std::endl;
		return 1;
	}
	CharStream stream(file);

	LexemAnalyzer analyser;
	LexemList lexems = analyser.parse(stream);

	std::cout << lexems << std::endl;
  return 0;
}
