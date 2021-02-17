/*------------------------------------------------------------------------*/
/*! \file checker.h
    \brief parses and checks the proof

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_CHECKER_H_
#define PACHECK2_SRC_CHECKER_H_
/*------------------------------------------------------------------------*/
#include "parser.h"
/*------------------------------------------------------------------------*/
// Global variables

/// determines whether target polynomial is checked
extern bool check_target;
/// determines wheter delete rules are executed or ignored
extern bool delete_mode;
/// determines whether we have found target
extern bool target_polynomial_inferences;
/// determines whether proof leads to a refutation
extern bool constant_one_polynomial_inferences;

/*------------------------------------------------------------------------*/
/**
    Reads the axiom from polys_file_name and checks the proof contained in
    rule_file_name

    @param polys_file_name  const char *
    @param rule_file_name  const char *
*/
void parse_and_check_proof(const char * polys_file_name,
                           const char * rule_file_name);

/**
    Reads the target from file_name

    @param file_name  const char *
*/
void parse_target_polynomial(const char * file_name);

/**
    Prints the statistics of proof checking to stdout
*/
void checker_statistics();

/**
    deallocates allocated objects
*/
void reset();

/*------------------------------------------------------------------------*/
#endif  // PACHECK2_SRC_CHECKER_H_
