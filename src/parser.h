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

// Functions
Polynomial parsePolynomial(const std::string& input);


Polynomial parsePolynomialExpr(std::vector<Token>& tokens, size_t& index);
Polynomial parseTerm(std::vector<Token>& tokens, size_t& index);
Polynomial parseFactor(std::vector<Token>& tokens, size_t& index);
Polynomial parseExpression(std::vector<Token>& tokens, size_t& i);
Monomial parseMonomialFromIdentifier(const std::string& varname, int exponent = 1);

// Interpreter
void processLine(std::string line);

/*------------------------------------------------------------------------*/
#endif  // PACHECK2_SRC_PARSER_H_
