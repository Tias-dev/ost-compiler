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
