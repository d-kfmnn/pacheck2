/*------------------------------------------------------------------------*/
/*! \file polynomial.h
    \brief contains arithmetic operations for polynomials

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_POLYNOMIAL_H_
#define PACHECK2_SRC_POLYNOMIAL_H_
/*------------------------------------------------------------------------*/
#include <list>
#include <deque>

#include "monomial.h"
/*------------------------------------------------------------------------*/

/** \class Polynomial
    This class is used to polynomials.
*/

class Polynomial {
  int idx;  ///< index as used in pac proofs

  int level = 1;  ///< level of polynomials needed for certificates

  Monomial * mon = 0;
  Polynomial * rest;
  unsigned siz = 0;

 public:

  Polynomial(Monomial * m, Polynomial * p);


  /** Getter for member idx

      @return integer
  */
  int get_idx() const {return idx;}

  /**
      Setter for idx

      @param idx_ integer
  */
  void set_idx(int idx_) {idx = idx_;}

  /** Getter for member level

      @return integer
  */
  int get_level() const {return level;}

  /**
      Setter for level

      @param level_ integer
  */
  void set_level(int level_) {level = level_;}

  /** Returns the leading monomial

      @return Monomial*
  */
  Monomial* get_lm() const {return mon;}




  /** Returns the leading term

      @return Term*
  */
  Term * get_lt() const {return mon->get_term();}

  /**
      Returns the number of monomials

      @return unsigned
  */
  unsigned size() const;

  /**
      Returns the size of the smallest term

      @return unsigned
  */
  unsigned min_term_size() const;

  /**
      Getter for rest

      @return Polynomial*
  */
  Polynomial* get_rest() const {return rest;}

  /**
      Returns the degree of a polynomial

      @return unsigned
  */
  unsigned degree() const;

  /**
      Returns whether the polynomial is the constant zero polynomial

      @return bool
  */
  bool is_constant_zero_poly() const;

  /**
      Returns whether the polynomial is the constant one polynomial

      @return bool
  */
  bool is_constant_one_poly() const;

  /**
      Copy routine

      @return A hard copy of the current polynomial
  */
  Polynomial * copy() const;

  /**
      Printing routine

      @param file Output file
      @param end if true we print trailing ";"
  */
  void print(FILE * file, bool end = 1) const;

  /** Destructor */
  ~Polynomial();
};
/*------------------------------------------------------------------------*/
// Polynomials are generated using a sorted array "mstack"

/**
    Deallocates mstack
*/
void deallocate_mstack();

/**
    Pushes a monomial to the stack such that mstack remains sorted

    @param t monomial to be added to the mstack
*/
void push_mstack(Monomial *t);


/**
    Generates a polynomial from mstack and clears mstack

    @param need_sorting indicates whether stack needs sorting first

    @return Polynomial*
*/
Polynomial * build_poly(bool need_sorting);


/*------------------------------------------------------------------------*/

/**
    Checks whether two polynomials are equal

    @param p1 Polynomial*
    @param p2 Polynomial*

    @return bool
*/
bool equal_polynomials(const Polynomial * p1, const Polynomial * p2);

/**
    Add two polynomials p1+p2

    @param p1 Polynomial*
    @param p2 Polynomial*

    @return Polynomial*, sum of p1+p2
*/
Polynomial * add_poly(const Polynomial *p1, const Polynomial *p2);

/**
    Multiplies two polynomials p1*p2

    @param p1 Polynomial*
    @param p2 Polynomial*

    @return Polynomial*, product of p1*p2
*/
Polynomial * multiply_poly(const Polynomial *p1, const Polynomial *p2);

/**
    Multiplies a polynomial p1 with -1

    @param p1: Polynomial*

    @return Polynomial*
*/
Polynomial * negate_poly(const Polynomial *p1);


Polynomial * zero_poly();


/*---------------------------------------------------------------------------*/
#endif  // PACHECK2_SRC_POLYNOMIAL_H_
