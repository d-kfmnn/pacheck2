/*------------------------------------------------------------------------*/
/*! \file variable.cpp
    \brief contains the class Var

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#include "variable.h"

int sort = 0;
/*------------------------------------------------------------------------*/
static unsigned num_variables;
static unsigned size_variables;
static Var ** variable_table;
/*------------------------------------------------------------------------*/
/**
    Enlarges the hash table
*/
static void enlarge_variables() {
  unsigned new_size_variables = size_variables ? 2*size_variables : 1;
  Var ** new_variable_table = new Var*[new_size_variables]();
  for (unsigned i = 0; i < size_variables; i++) {
    for (Var * v = variable_table[i], * n; v; v = n) {
      unsigned h = v->get_hash() &(new_size_variables - 1);
      n = v->get_next();
      v->set_next(new_variable_table[h]);
      new_variable_table[h] = v;
    }
  }
  delete[] variable_table;
  variable_table = new_variable_table;
  size_variables = new_size_variables;
}

/*------------------------------------------------------------------------*/

Var * new_variable(const char * name, bool new_var_allowed) {
  std:: string name_s(name);
  if (num_variables == size_variables) enlarge_variables();
  const uint64_t hash = hash_string(name_s);
  const unsigned h = hash &(size_variables - 1);

  Var * res = variable_table[h];
  while (res) {
    if (res->get_name_string().compare(name_s)== 0) break;
    res = res->get_next();
  }

  if (!res && !new_var_allowed) {
    fprintf(stderr,
      "*** 'pacheck' error: Var '%s' is not contained in ideal.\n", name);
    fflush(stderr);
    exit(1);
  }
  if (!res) {
    res = new Var(name_s, ++num_variables, hash, variable_table[h]);
    variable_table[h] = res;
  } else {
    res->inc_count();
  }
  return res;
}

/*------------------------------------------------------------------------*/

int cmp_variable(const Var *a, const Var *b) {
  if (a == b) return 0;
  if     (sort == 0 && strcmp(a->get_name(), b->get_name()) > 0) return 1;
  else if (sort == 1 && strcmp(a->get_name(), b->get_name()) < 0) return 1;
  else if (sort == 2 && a->get_level() < b->get_level()) return 1;
  else if (sort == 3 && a->get_level() > b->get_level()) return 1;
  else
    return -1;
}

/*------------------------------------------------------------------------*/

void deallocate_variables() {
  for (unsigned i = 0; i < size_variables; i++) {
    for (Var * v = variable_table[i], * n; v; v = n) {
      n = v->get_next();
      delete(v);
    }
  }
  delete [] variable_table;
}
