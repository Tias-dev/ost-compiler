#include "AST.hpp"
#include "BreakPointer.hpp"
#include "CharStream.hpp"
#include "Compiler.hpp"
#include "FilePosition.hpp"
#include "Tokenizer.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
 
int main(int argc, char * argw[]) {
	if(argc < 3) 
		throw std::invalid_argument("Usage: ost <program>.ost <output dir> <library dir>");
	std::string fileName = argw[1];

	std::string outDir = argw[2];
	if(outDir[outDir.size() - 1] != '/') 
		outDir.push_back('/');
	
	globals::libDir = argw[3];
	if(globals::libDir[globals::libDir.size() - 1] != '/') 
		globals::libDir.push_back('/');

	if(globals::enableBreakpoints) 
		globals::breakpointer = new FileBreakpointer{fileName};

	std::fstream file(fileName);
	if(!file.is_open()) 
		throw std::invalid_argument("Can't open file");
	
	CharStream stream(file);

	auto tokenizer = token::Tokenizer{};
	FileRoller roller{std::make_shared<std::string>(fileName)};
	auto tokens = tokenizer.parse(stream, roller);
	file.close();

	if(globals::printDebugInfo) {
		std::cout << "Tokenizer output:" << std::endl;
		for(auto& token : tokens) 
			std::cout << token.toString() << std::endl;
		std::cout << "-----------------------------" << std::endl << std::endl;
	}

	ast::Tree astTree{tokens, fileName};
	if(globals::printDebugInfo) {
		std::cout << "Created AST tree:" << std::endl;
		astTree.print();
		std::cout << "-----------------------------" << std::endl << std::endl;
		std::cout << "Names table:" << std::endl;
		ast::MT::printNamesTable();
	}
	globals::foutName = strfast() << outDir << astTree.getTreeName() << ".tu4";

	std::ofstream fout(globals::foutName);
	compiler::commands_type commands = astTree.to4();
	for(auto& command : commands) 
		fout << command << std::endl;
	

  return 0;
}
