#include "Line.hpp"
#include "Tu4Runner.hpp"
#include <iostream>
#include <string>


void usage() {
	std::cout << "Usage: tu4run <program name>.tu4 [OPTIONS]" << std::endl;
}


int main(int argc, char * argw[]) {
	if(argc < 2) {
		usage();
		return 1;
	}

	std::string fileName = argw[1];
	std::string s;
	std::getline(std::cin, s);

	auto runner = tu4run::initRunner(fileName, s);
		
	size_t steps = runner->loop();


	std::cout << "Steps: " << steps << " Out line:" << std::endl;
	std::cout << runner->line() << std::endl;

	return 0;
}
