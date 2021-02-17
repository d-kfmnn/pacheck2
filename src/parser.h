/*------------------------------------------------------------------------*/
/*! \file parser.h
    \brief core functions for parsing

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_PARSER_H_
#define PACHECK2_SRC_PARSER_H_
/*------------------------------------------------------------------------*/
#include <vector>

#include "inference.h"
/*------------------------------------------------------------------------*/
/// name of the input file
extern const char * parse_file_name;
/// input file
extern FILE * parse_file;
/// determines the number of lines while parsing
extern unsigned lineno;
/// determines the number of characters while parsing
extern unsigned charno;
/// keeps track of the current lineno
extern unsigned lineno_at_start_of_last_token;

/**
    deallocates the buffer, which is used for reading
*/
void deallocate_buffer();
/*------------------------------------------------------------------------*/
typedef const char * Token;

/**
    determins whether stored token is ';'

    @return bool
*/
bool is_semicolon_token();

/**
    determins whether stored token is ','

    @return bool
*/
bool is_comma_token();

/**
    determins whether stored token is '+'

    @return bool
*/
bool is_plus_token();

/**
    determins whether stored token is '*'

    @return bool
*/
bool is_multiply_token();

/**
    determins whether stored token is '('

    @return bool
*/
bool is_open_parenthesis_token();

/**
    determins whether stored token is ')'

    @return bool
*/
bool is_close_parenthesis_token();

/**
    determins whether stored token is '='

    @return bool
*/
bool is_extension_token();

/**
    determins whether stored token is '%'

    @return bool
*/
bool is_lin_combi_token();

/**
    determins whether stored token is 'd'

    @return bool
*/
bool is_delete_token();

/**
    determins whether the next token  token is '\n'

    @return bool
*/
bool following_token_is_EOF();

/**
    returns the current token

    @return Token
*/
Token get_token();

/**
    reads and returns a Token

    @return Token
*/
Token next_token();

/**
    prints an error message to stderr and kills the program

    @param msg const char *
*/
void parse_error(const char * msg, ...);

/**
    parses a polynomial, new variables are only allowed if new_var_allowed = 1

    @param not_new_allowed bool

    @return Polynomial *
*/
Polynomial * parse_polynomial(bool new_var_allowed = 1);

/**
    parses an index

    @return unsigned
*/
unsigned parse_index();

/*------------------------------------------------------------------------*/
#endif  // PACHECK2_SRC_PARSER_H_
