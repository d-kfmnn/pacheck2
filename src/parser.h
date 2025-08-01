/*------------------------------------------------------------------------*/
/*! \file parser.h
    \brief core functions for parsing

  Part of Pacheck 3.0 : PAC proof checker.
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_PARSER_H_
#define PACHECK2_SRC_PARSER_H_
/*------------------------------------------------------------------------*/
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include "polynomial.hpp"
/*------------------------------------------------------------------------*/
/// name of the input file

extern std::unordered_map<int, Polynomial> id_to_poly;
extern std::vector<std::pair<std::string, int>> substitution_stack;

extern std::unordered_map<std::string, std::unordered_set<int>> declared_roots;



enum class TokenType { Number, Identifier, Operator, End };

struct Token {
    TokenType type;
    std::string value;
};
/*------------------------------------------------------------------------*/
// Functions

Polynomial parseFactor(std::vector<Token>& tokens, size_t& index);

// Interpreter
void processLine(std::string line, int lineno);

void printFinalStatistics();

/*------------------------------------------------------------------------*/
#endif  // PACHECK2_SRC_PARSER_H_
