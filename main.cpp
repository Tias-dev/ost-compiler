#include "AST.hpp"
#include "CharStream.hpp"
#include "Tokenizer.hpp"
#include "exception.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
 
int main(int argc, char * argw[]) {
	std::string fileName = "./testProgram.ost";
	std::fstream file(fileName);
	if(!file.is_open()) {
		std::cout << "Can't open file" << std::endl;
		return 1;
	}
	CharStream stream(file);

	auto tokenizer = token::Tokenizer{};
	auto tokens = tokenizer.parse(stream);
	file.close();

	for(auto& token : tokens) 
		std::cout << token.toString() << std::endl;
	std::cout << "-----------------------------" << std::endl;
	
	try {
		ast::Tree astTree{tokens};
		std::cout << "Current MT names table:" << std::endl;
		ast::MT::printNamesTable();
		astTree.print();
	} catch (error::PositionErrorBase & e) {

		std::cout << "Error: " << e.what() << std::endl;
		size_t position = e.position();
		size_t width = 60;
		std::fstream fileRunner(fileName);
		for(size_t i = 0; i < position - std::min(position, width/2); ++i) 
			fileRunner.get();

		char c;
		for(size_t i = 0; i < width; ++i)  {
			c = fileRunner.get();
			if(isspace(c)) 
				c = ' ';

			std::cout << c;
		}
		std::cout << std::endl;
		for(size_t i = 0; i < width/2; ++i) 
			std::cout << " ";	
		std::cout << "^" << std::endl;
		
	}
  return 0;
}
