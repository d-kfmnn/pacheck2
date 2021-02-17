/*------------------------------------------------------------------------*/
/*! \file signal_statistics.h
    \brief used to handle signals, messages and statistics

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_SIGNAL_STATISTICS_H_
#define PACHECK2_SRC_SIGNAL_STATISTICS_H_
/*------------------------------------------------------------------------*/
#include <stdarg.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <inttypes.h>

#include <iostream>
/*------------------------------------------------------------------------*/
// Global variables

/// Level of output verbosity, ranges from 0 to 4
extern int verbose;

/// counts the number of simoultaneously allocated terms
extern unsigned max_terms;
/// counts all terms
extern unsigned total_terms;
/// counts how often we already find a term
extern unsigned hits_terms;
/// counts the number of searches of terms
extern unsigned searched_terms;
/// counts the number of collisions
extern unsigned collisions_terms;

/// maximum number of inferences
extern unsigned max_inferences;
/// counts the number of searches for inferences
extern unsigned searched_inferences;
/// counts the number of collisions
extern unsigned collisions_inferences;
/*------------------------------------------------------------------------*/
/**
    Initialize all signals
*/
void init_all_signal_handers();


/**
    Resets all signal handlers
*/
void reset_all_signal_handlers();

/*------------------------------------------------------------------------*/
/**
    Prints a message to stdout

    @param char* fmt message
*/
void msg(const char *fmt, ...);

/**
    Prints an error message to stderr and exits the program

    @param char* fmt message
*/
void die(const char *fmt, ...);
/*------------------------------------------------------------------------*/
/**
    Updates global statistics, such as degree, max polynomial length, etc.

    @param degree int
    @param size int
*/

void update_statistics_for_newly_added_polynomial(int degree, int size);
/*------------------------------------------------------------------------*/

/**
    Print statistics of maximum memory and used process time depending on
    selected modus
*/
void print_statistics(unsigned original_inferences,
  unsigned extension_inferences,
  unsigned lin_comb_inferences,
  unsigned deletion_inferences,
  unsigned num_inference_rules,
  unsigned addition_operations,
  unsigned multiplication_operations);

#endif  // PACHECK2_SRC_SIGNAL_STATISTICS_H_
