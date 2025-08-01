/*------------------------------------------------------------------------*/
/*! \file pacheck.cpp
    Main file

    Part of Pacheck 3.0 : PAC proof checker.
*/
/*------------------------------------------------------------------------*/
#include "parser.h"
#include <iostream>
#include <fstream>
/*------------------------------------------------------------------------*/
#define VERSION "3.0"
/*------------------------------------------------------------------------*/

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./pacheck <input_file>" << std::endl;
    return 1;
  }

  std::cout << "==========================================" << std::endl;
  std::cout << "         Pacheck Proof Checker " << VERSION << std::endl;
  std::cout << "==========================================" << std::endl;

  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
    return 1;
  }
  std::cout << "Pacheck reads proof from file: " << argv[1] << std::endl;

  std::string line;
  int i = 1;
  while (std::getline(infile, line)) {
    if (line.empty() || line[0] == 'c') continue;  // skip empty and comment lines
    processLine(line, i++);
  }

  printFinalStatistics();
  std::cout << "Proof check completed successfully." << std::endl;

  infile.close();
  return 0;
}

