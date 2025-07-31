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

  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
    return 1;
  }

  std::string line;
  while (std::getline(infile, line)) {
    if (line.empty() || line[0] == 'c') continue;  // skip empty and comment lines
    processLine(line);
  }

  infile.close();
  return 0;
}

