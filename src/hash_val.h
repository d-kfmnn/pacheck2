/*------------------------------------------------------------------------*/
/*! \file hash_val.h
    \brief contains functions used to compute hash values for variables

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_HASH_VAL_H_
#define PACHECK2_SRC_HASH_VAL_H_
/*------------------------------------------------------------------------*/
#include <assert.h>

#include <string>
/*------------------------------------------------------------------------*/
/**
    Fills a 32-bit array with 64-bit random numbers
*/
void init_nonces();

/**
    Returns the 64-bit random number in our array of random numbers

    @param index size_t

    @return returns the random number stored in nonces[index]
*/
uint64_t get_nonces_entry(size_t index);

/**
    Computes the hash value for the given string

    @param str a const std::string

    @return a uint64_t computed hash value for the input string
*/
uint64_t hash_string(const std::string& str);

#endif  // PACHECK2_SRC_HASH_VAL_H_
