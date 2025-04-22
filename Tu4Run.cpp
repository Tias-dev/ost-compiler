#include "Compiler.hpp"
#include "Line.hpp"
#include "Tu4Command.hpp"
#include "Tu4Runner.hpp"
#include "exception.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace tu4run;

void usage() {
	std::cout << "Usage: tu4run <program name>.tu4 [OPTIONS]" << std::endl;
}

int main(int argc, char * argw[]) {
	if(argc < 2) {
		usage();
		return 1;
	}

	std::ifstream file(argw[1]);
	if(file.bad()) 
		throw std::invalid_argument(strfast() << "Can't open file by [" << argw[1] << "] path");
		
	compiler::Commands<size_t, char> commands;
	try {
		while(true) {
			commands.push_back(load<size_t, char>(file));
		}
	} catch (error::UnexpectedFileEnd) { }
	std::string s;
	std::getline(std::cin, s);

	Line<char> line(s);
	Tu4Runner<size_t, char> runner(line, commands);
	size_t steps = runner.loop();
	std::cout << "Steps: " << steps << " Out line:" << std::endl;
	std::cout << line << std::endl;

	return 0;
}
