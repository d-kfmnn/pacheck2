/*------------------------------------------------------------------------*/
/*! \file pattern.h
    \brief core functions for pattern matching

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2025 Daniela Kaufmann, TU Wien
*/
/*------------------------------------------------------------------------*/
#include "parser.h"
#include "inference.h"

extern unsigned new_patterns_count;
extern unsigned apply_patterns_count;
/*------------------------------------------------------------------------*/
// Global variables

void parse_pattern();

void polynomials_do_not_match(
  unsigned index, const Polynomial *actual, const Polynomial *expected,
  unsigned rule_line, unsigned polynomial_line) ;