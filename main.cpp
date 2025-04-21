#include "AST.hpp"
#include "CharStream.hpp"
#include "Compiler.hpp"
#include "Tokenizer.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <cctype>
#include <fstream>
#include <iostream>
 
int main(int argc, char * argw[]) {
	std::string fileName = (argc > 1 ? argw[1] : "./testProgram2.ost");
	std::fstream file(fileName);
	if(!file.is_open()) {
		std::cout << "Can't open file" << std::endl;
		return 1;
	}
	CharStream stream(file);

	auto tokenizer = token::Tokenizer{};
	auto tokens = tokenizer.parse(stream);
	file.close();

	ast::Tree astTree{tokens, fileName};
	if(globals::printDebugInfo) {
		std::cout << "Tokenizer output:" << std::endl;
		for(auto& token : tokens) 
			std::cout << token.toString() << std::endl;
		std::cout << "-----------------------------" << std::endl << std::endl;
		std::cout << "Created AST tree:" << std::endl;
		astTree.print();
		std::cout << "-----------------------------" << std::endl << std::endl;
	}
	globals::foutName = strfast() << "./" << astTree.getTreeName() << ".tu4";

	std::ofstream fout(globals::foutName);
	compiler::commands_type commands = astTree.to4();
	for(auto& command : commands) 
		fout << command << std::endl;
	

  return 0;
}
