/*------------------------------------------------------------------------*/
/*! \file inference.h
    \brief contains the class Inference and futher functions to manipulate
    inferences

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#ifndef PACHECK2_SRC_INFERENCE_H_
#define PACHECK2_SRC_INFERENCE_H_
/*------------------------------------------------------------------------*/
#include "polynomial.h"
/*------------------------------------------------------------------------*/

class Inference {
  unsigned id;                     ///< unique identification index
  const Polynomial * conclusion;   ///< produced / added polynomial
  Inference * chronological_next;  ///< next doubly linked list
  Inference * chronological_prev;  ///< prev doubly linked list

 public:
  Inference * collision_chain_link;  ///< for hash table

  /** Constructor

     @param conclusion Polynomial*
     @param index unsigned
  */
  Inference(const Polynomial * conclusion, unsigned index);

  /**
    updates chronological_prev and chronological_next when inference is removed
  */
  void update_links_to_remove_inference();

/** Destructor */
  ~Inference();

  /** Returns the index id

      @return unsigned
  */
  unsigned get_id() const {return id;}

   /** Returns the conclusion

       @return const Polynomial*
   */
  const Polynomial* get_conclusion() const {return conclusion;}

   /** Returns chronological_next

       @return  Inference *
   */
  Inference * get_chronological_next() const {return chronological_next;}

   /**
       Setter for chronological_next

       @param inf Inference *
   */
  void set_chronological_next(Inference * inf) {chronological_next = inf;}

   /** Returns chronological_prev

       @return  Inference *
   */
  Inference * get_chronological_prev() const {return chronological_prev;}

   /**
       Setter for chronological_prev

       @param inf Inference *
   */
  void set_chronological_prev(Inference * inf) {chronological_prev = inf;}

   /** Returns collision_chain_link

       @return  Inference *
   */
  Inference * get_collision_chain_link() const {return collision_chain_link;}

   /**
       Setter for collision_chain_link

       @param inf Inference *
   */
  void set_collision_chain_link(Inference * inf) {collision_chain_link = inf;}
};

/*------------------------------------------------------------------------*/

/**
    Allocates a new inference and inserts it to the hash table

    @param index unsigned
    @param conclusion Polynomial*

    @return Inference*
*/
Inference * new_inference(unsigned index, const Polynomial * conclusion);

/**
    Searches if inference with given index is contained in hash table

    @param index unsigned

    @return Inference*
*/
const Inference * find_inference_index(unsigned index);

/**
    Removes inference with given index from hash table

    @param index unsigned
*/
void delete_inference_by_index(unsigned index);

/**
    Deletes all inferences
*/
void delete_inferences();



/*------------------------------------------------------------------------*/
#endif  // PACHECK2_SRC_INFERENCE_H_
