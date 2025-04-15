#include "CharStream.hpp"
#include "Tokenizer.hpp"
#include <cctype>
#include <fstream>
#include <iostream>
#include <variant>
 
int main(int argc, char * argw[]) {
	std::fstream file("./testProgram.ost");
	if(!file.is_open()) {
		std::cout << "Can't open file" << std::endl;
		return 1;
	}
	CharStream stream(file);

	auto tokenizer = token::Tokenizer{};
	auto tokens = tokenizer.parse(stream);

	for(auto& token : tokens) 
		std::cout << std::visit([](const auto & t) {return t.toString();}, token) << std::endl;
	

  return 0;
}
