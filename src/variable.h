/*------------------------------------------------------------------------*/
/*! \file variable.h
    \brief contains the class Var

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_VARIABLE_H_
#define PACHECK2_SRC_VARIABLE_H_
/*------------------------------------------------------------------------*/
#include <string>
#include <cstring>

#include "hash_val.h"
#include "signal_statistics.h"

/// determines which monomial ordering is used
extern int sort;
/*------------------------------------------------------------------------*/

/** \class Var
    represents a variable is assigned to a gate(see <gate.h>) and is
    used to represent variables in terms, see <term.h>
*/

class Var {
    /// name of variable
    const std::string name;
    /// Increasing value that indicates the order of the variable
    const int level;
    /// Hash value of variables, used for storing terms
    unsigned hash;
    /// corresponding value used to relate AIG gates to Gate class
    int count = 1;
    /// collision chain link
    Var * next;

 public:
  /** Constructor

     @param name_ name
     @param level_ level
     @param hash_ hash value
     @param n_  Var* next

  */
  Var(std::string name_, int level_, int hash_, Var * n_):
     name(name_),  level(level_), hash(hash_), next(n_) {}

   /** Getter for member name, and converts string to char*

       @return const char *
   */
  const char * get_name() const {return name.c_str();}

  /** Getter for member name

      @return std::string
  */
  const std::string get_name_string() const {return name;}

  /** Getter for member hash

      @return integer
  */
  int get_hash() const {return hash;}

  /** Getter for member level

      @return integer
  */
  int get_level() const {return level;}

  /** Getter for member count

      @return integer
  */
  int get_count() const {return count;}

  /** Increments member count
  */
  void inc_count() {++count;}

  /** Getter for member next

      @return Var *
  */
  Var * get_next() const {return next;}

  /** Setter for member next

      @param n Var*
  */
  void set_next(Var *n) {next = n;}
};


/**
    Builds a term, where variable is added at the front of rest

    @param name const char*
    @param not_new_allowed bool, true if no new variable is allowed to be allocated

    @return Term*
*/
Var * new_variable(const char * name, bool new_var_allowed);

/**
    Compares to variables according to set sort function

    @param a const Var*
    @param b const Var*

    @return 1 if a > b, 0 if a = b, -1 if a < b
*/
int cmp_variable(const Var* a, const Var* b);

/**
    Deallocates the hash table "term_table"
*/
void deallocate_variables();


#endif  // PACHECK2_SRC_VARIABLE_H_
