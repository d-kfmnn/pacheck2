/*------------------------------------------------------------------------*/
/*! \file term.cpp
    \brief contains the class Term and further functions to
    manipulate terms

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#include "term.h"
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/

Term::Term(const Var * _v,  Term * _r, uint64_t _hash, Term * _n):
  variable(_v), ref(1), hash(_hash), next(_n) {
  if (_r) rest = _r->copy();
  else
    rest = 0;
}

/*------------------------------------------------------------------------*/

Term * Term::copy() {
  assert(ref > 0);
  ++ref;
  assert(ref);
  return this;
}

/*------------------------------------------------------------------------*/

void Term::print(FILE * file) const {
  const Term * tmp = this;
  if (!tmp) fputc_unlocked('0', file);
  while (tmp) {
    fputs_unlocked(tmp->get_var_name(), file);
    tmp = tmp->get_rest();
    if (tmp) fputc_unlocked('*', file);
  }
}

/*------------------------------------------------------------------------*/

unsigned Term::size() const {
  const Term * tmp = this;
  if (!tmp) return 0;

  int i = 0;
  while (tmp) {
    i++;
    tmp = tmp->get_rest();
  }
  return i;
}

/*------------------------------------------------------------------------*/

int Term::cmp(const Term *t) const {
  const Term * tmp1 = this;
  if (!tmp1) return -1;
  const Term * tmp2 = t;
  if (!tmp2) return 1;

  if (tmp1 == tmp2) return 0;

  while (tmp1 && tmp2 && tmp1->get_var() == tmp2->get_var()) {
      tmp1 = tmp1->get_rest();
      tmp2 = tmp2->get_rest();
  }

  if (tmp1 && tmp2) return cmp_variable(tmp1->get_var(), tmp2->get_var());
  else if (tmp1) return 1;
  else
    return -1;
}

/*------------------------------------------------------------------------*/

static unsigned size_terms;
static unsigned current_terms;
static Term ** term_table;

/*------------------------------------------------------------------------*/

/**
    Compute hash_values

    @param variable Variable*
    @param rest Term*

    @return computed hash value for the term(variable, rest)
*/
static uint64_t compute_hash_term(const Var * variable, const Term * rest) {
  assert(variable);
  uint64_t res = rest ? rest->get_hash() : 0;
  res *= get_nonces_entry(0);
  res += variable->get_hash();
  res *= get_nonces_entry(1);
  return res;
}

/*------------------------------------------------------------------------*/

/**
    Enlarges the hash table
*/
static void enlarge_terms() {
  unsigned new_size_terms = size_terms ? 2*size_terms : 1;
  Term ** new_term_table = new Term*[new_size_terms]();
  for (unsigned i = 0; i < size_terms; i++) {
    for (Term * m = term_table[i], * n; m; m = n) {
      uint64_t h = m->get_hash() &(new_size_terms - 1);
      n = m->get_next();
      m->set_next(new_term_table[h]);
      new_term_table[h] = m;
    }
  }
  delete[] term_table;
  term_table = new_term_table;
  size_terms = new_size_terms;
}

/*------------------------------------------------------------------------*/

Term * new_term(const Var * variable, Term * rest) {
  if (current_terms == size_terms) enlarge_terms();
  const uint64_t hash = compute_hash_term(variable, rest);
  const uint64_t h = hash &(size_terms - 1);
  searched_terms++;

  Term * res;
  for (res = term_table[h];
       res &&(res->get_var() != variable || res->get_rest() != rest);
       res = res->get_next()) {
         collisions_terms++;
       }

  if (res) {
    res->inc_ref();  // here we extend that we found term once more
    hits_terms++;
  } else {
    res = new Term(variable, rest, hash, term_table[h]);
    term_table[h] = res;
    current_terms++;
    total_terms++;
    if (current_terms > max_terms) max_terms = current_terms;
  }
  return res;
}

/*------------------------------------------------------------------------*/

void deallocate_term(Term * t) {
  while (t) {
    assert(t->get_ref() > 0);
    if (t->dec_ref() > 0) break;  // t is still used
    Term * rest = t->get_rest();
    const uint64_t h = t->get_hash() &(size_terms - 1);
    Term * p = term_table[h];
    if (p == t) { term_table[h] = t->get_next();
    } else {
      Term * p2;
      while ((p2 = p->get_next()) != t) p = p2;
      p->set_next(t->get_next());
    }

    assert(current_terms);
    current_terms--;
    delete(t);
    t = rest;
  }
}

/*------------------------------------------------------------------------*/

void deallocate_terms() {
  for (unsigned i = 0; i < size_terms; i++) {
    for (Term * m = term_table[i], *n; m; m = n) {
      n = m->get_next();
      assert(current_terms);
      current_terms--;

      delete(m);
    }
  }
  delete[] term_table;
}

/*------------------------------------------------------------------------*/
static size_t size_var_list;  ///< size of mstack
static size_t num_var_list = 0;  ///< number of elements in mstack
static const Var ** var_list;  ///< Var ** used for building poly
/*------------------------------------------------------------------------*/

static void enlarge_var_list() {
  size_t new_size_var_list = size_var_list ? 2*size_var_list : 1;

  const Var ** newArr = new const Var*[new_size_var_list];
  memcpy( newArr, var_list, size_var_list * sizeof(Var*));
  delete [] var_list;
  var_list = newArr;
  size_var_list = new_size_var_list;
}

/*------------------------------------------------------------------------*/

static void clear_var_list() { num_var_list = 0; }

/*------------------------------------------------------------------------*/

void deallocate_var_list() { delete[](var_list); }

/*------------------------------------------------------------------------*/

static bool var_list_is_empty() { return num_var_list == 0;}
/*------------------------------------------------------------------------*/

/**
    Pushes a variable to the end of the variable stack,
    thus variables have to be pushed in order

    @param v Var*
*/
static void push_var_list_end(const Var* v) {
  if (size_var_list == num_var_list) enlarge_var_list();
  assert(v);
  var_list[num_var_list++] = v;
}

/*------------------------------------------------------------------------*/

void push_var_list(const Var *v) {
  if (size_var_list == num_var_list) enlarge_var_list();
  if (var_list_is_empty()) { var_list[num_var_list++] = v;
  } else {
      assert(num_var_list > 0);
      int i = num_var_list-1;
      int cmp = -1;
      const Var * tmp = 0;

      while (i >= 0) {
        tmp = var_list[i];
        cmp = cmp_variable(tmp, v);
        if (cmp > 0) break;
        else if (cmp == 0) return;
        i--;
      }
      for (int j = num_var_list; j > i+1; j--) {
        var_list[j] = var_list[j-1];
      }

      var_list[i+1] = v;
      num_var_list++;
  }
}


/*------------------------------------------------------------------------*/

Term * build_term_from_list() {
  Term * res = 0;
  int i = num_var_list;
  while (i > 0) {
    const Var * v = var_list[--i];
    Term * t = new_term(v, res);
    deallocate_term(res);
    res = t;
  }
  clear_var_list();
  return res;
}

/*------------------------------------------------------------------------*/

Term * multiply_term(Term * t1, Term * t2) {
  if (!t1 || !t2) return 0;
  if (t1 == t2) return t1->copy();

  const Term * tmp1 = t1;
  const Term * tmp2 = t2;


  while (tmp1 && tmp2) {
    if (tmp1 == tmp2) {
      tmp2 = 0;
      break;
    }

    if (tmp1->get_var() == tmp2->get_var()) {
      push_var_list_end(tmp1->get_var());
      tmp1 = tmp1->get_rest();
      tmp2 = tmp2->get_rest();
    } else if (cmp_variable(tmp1->get_var(), tmp2->get_var()) > 0) {
      push_var_list_end(tmp1->get_var());
      tmp1 = tmp1->get_rest();
    } else {
      push_var_list_end(tmp2->get_var());
      tmp2 = tmp2->get_rest();
    }
  }

  while (tmp1) {
    push_var_list_end(tmp1->get_var());
    tmp1 = tmp1->get_rest();
  }

  while (tmp2) {
    push_var_list_end(tmp2->get_var());
    tmp2 = tmp2->get_rest();
  }
  Term * t = build_term_from_list();

  return t;
}

/*------------------------------------------------------------------------*/
