/*------------------------------------------------------------------------*/
/*! \file inference.cpp
    \brief contains the class Inference and futher functions to manipulate
    inferences

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#include "inference.h"
/*------------------------------------------------------------------------*/

Inference::Inference(const Polynomial * conclusion, unsigned index):
    id(index), conclusion(conclusion),
    chronological_next(0), collision_chain_link(0) {}

/*------------------------------------------------------------------------*/

void Inference::update_links_to_remove_inference() {
  if (chronological_prev)
        chronological_prev->chronological_next = chronological_next;

  if (chronological_next)
        chronological_next->chronological_prev = chronological_prev;
}

/*------------------------------------------------------------------------*/

Inference::~Inference() {
  delete(conclusion);
}
/*------------------------------------------------------------------------*/
// Global variables
unsigned searched_inferences;
unsigned collisions_inferences;

// Local variables
static unsigned size_inferences;
static Inference ** inference_table;
static Inference * first_inference;
static Inference * last_inference;
static unsigned num_inferences;

/*------------------------------------------------------------------------*/

static void enlarge_inferences() {
  unsigned new_size_inferences = size_inferences ? 2*size_inferences : 1;
  Inference ** new_inference_table = new Inference* [new_size_inferences]();
  for (Inference * p = last_inference; p; p = p->get_chronological_prev()) {
    unsigned h = p->get_id() &(new_size_inferences - 1);
    Inference * m = new_inference_table[h];
    p->set_collision_chain_link(m);
    new_inference_table[h] = p;
  }
  delete[](inference_table);
  inference_table = new_inference_table;
  size_inferences = new_size_inferences;
}

/*------------------------------------------------------------------------*/

Inference * new_inference(unsigned index, const Polynomial * conclusion) {
  if (num_inferences == size_inferences) enlarge_inferences();
  const unsigned h = index &(size_inferences - 1);
  Inference ** p, * res;
  searched_inferences++;
  for (p = inference_table + h;
       p &&(res = *p);
       p = &res->collision_chain_link)
    collisions_inferences++;
  res = new Inference(conclusion, index);
  if (!last_inference) first_inference = res;
  else
    last_inference->set_chronological_next(res);
  res->set_chronological_prev(last_inference);
  last_inference = res;

  num_inferences++;
  max_inferences++;
  if (p) *p = res;

  update_statistics_for_newly_added_polynomial(
    conclusion->degree(), conclusion->size());
  return res;
}

/*------------------------------------------------------------------------*/

static Inference * find_inference_index_non_const(unsigned index) {
  if (!size_inferences) return 0;
  const unsigned h = index &(size_inferences - 1);

  searched_inferences++;
  Inference * res;
  for (res = inference_table[h];
       res &&(res->get_id() != index);
       res = res->get_collision_chain_link())
    collisions_inferences++;
  return res;
}

/*------------------------------------------------------------------------*/
const Inference * find_inference_index(unsigned index) {
  const Inference * i = find_inference_index_non_const(index);
  return i;
}

/*------------------------------------------------------------------------*/

void delete_inference_by_index(unsigned index) {
  Inference * i = find_inference_index_non_const(index);
  if (!i) {
    fprintf(stdout, "\n");
    msg("WARNING: cannot delete inference with index %i", index);
    msg("         inference %i does not exist\n", index);
    return;
  }

  if (first_inference == i) first_inference = i->get_chronological_next();
  if (last_inference == i)  last_inference = i->get_chronological_prev();

  i->update_links_to_remove_inference();

  const unsigned h = i->get_id() &(size_inferences - 1);
  Inference ** p = inference_table + h, * res;
  while ((res = *p) != i) assert(res), p = &res->collision_chain_link;
  *p = res->get_collision_chain_link();

  num_inferences--;
  delete(i);
}

/*------------------------------------------------------------------------*/

void delete_inferences() {
  for (Inference * p = first_inference, * n; p; p = n) {
    n = p->get_chronological_next();
    delete(p);
  }
  delete [] inference_table;
}
