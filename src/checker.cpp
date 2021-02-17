/*------------------------------------------------------------------------*/
/*! \file checker.cpp
    \brief parses and checks the proof

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#include <vector>

#include "checker.h"
/*------------------------------------------------------------------------*/
// Global variables
bool check_target = 1;
bool delete_mode = 1;
bool target_polynomial_inferences = 0;
bool constant_one_polynomial_inferences = 0;

/*------------------------------------------------------------------------*/
// Local variables

/// stores the target
static Polynomial * target;
/// counts the number of inference rules
static unsigned num_inference_rules;
/// counts the number of axioms
static unsigned original_inferences;
/// counts the number of extension rules
static unsigned extension_inferences;
/// counts the number of linear combination rules
static unsigned lin_comb_inferences;
/// counts the number of deletion rules
static unsigned deletion_inferences;
/// counts the number of addition operations
static unsigned addition_operations;
/// counts the number of multiplication operations
static unsigned multiplication_operations;

/*------------------------------------------------------------------------*/
/**
    Opens the file

    @param file_name  const char *
*/
static void init_parsing(const char * file_name) {
  parse_file_name = file_name;
  parse_file = fopen(file_name, "r");
  if (!parse_file) die("can not open '%s' for reading", file_name);

  lineno = 1;
  charno = 0;
}
/*------------------------------------------------------------------------*/
/**
    Closes the parse file
*/
static void reset_parsing() {
  if (fclose(parse_file))
    die("failed to close '%s'", parse_file_name);
  msg("read %i bytes from '%s'", charno, parse_file_name);
  msg("");
}

/***************************************************************************/
/**
    Prints an error message that polynomial with p_index was not found

    @param index unsigned
    @param p_index unsigned
    @param rule_line unsigned
*/
static void polynomial_not_found(
  unsigned index, unsigned p_index, unsigned rule_line) {
  fflush(stdout);

  fprintf(stderr, "*** 'pacheck' error in rule with index %i ", index);
  fprintf(stderr, " in '%s' line %i: polynomial with index %i not found",
        parse_file_name, rule_line, p_index);

  if (delete_mode) {
    fputs("\ndelete mode is ON - try '--no-delete'", stderr);
  }

  fputc('\n', stderr);
  fflush(stderr);
  exit(1);
}

/*------------------------------------------------------------------------*/
/**
    Prints an error message that polynomials 'actual' and 'expected' do not match

    @param index unsigned
    @param actual const Polynomial *
    @param expected const Polynomial *
    @param rule_line unsigned
    @param polynomial_line unsigned
*/
static void polynomials_do_not_match(
  unsigned index, const Polynomial * actual, const Polynomial * expected,
  unsigned rule_line, unsigned polynomial_line) {
  fflush(stdout);
  fprintf(stderr, "*** 'pacheck' error in rule with index %i ", index);

  fprintf(stderr,
    " in '%s' line %i: conclusion polynomial", parse_file_name, rule_line);

  if (rule_line != polynomial_line)
    fprintf(stderr, " line %i", polynomial_line);

  fputs(":\n", stderr);
  actual->print(stderr);
  fputs("\ndoes not match expected result:\n", stderr);
  expected->print(stderr);
  fputc('\n', stderr);
  fprintf(stdout, "%i %i", actual->size(), expected->size());
  fflush(stderr);
  exit(1);
}

/***************************************************************************/
/**
    Checks whether p is a single new variable

    @param p const Polynomial *

    @return true if p is a valid extension variable
*/
static bool check_for_valid_extension_var(const Polynomial * p) {
  if (p->size() > 1) return 0;
  if (mpz_cmp_si(p->get_lm()->coeff, 1) != 0) return 0;

  Term *t = p->get_lt();
  if (!t) return 0;
  if (t->size() > 1) return 0;
  if (t->get_var()->get_count() > 1) return 0;

  return 1;
}

/*------------------------------------------------------------------------*/
/**
    Checks whether p is a valid extension, i.e. p*p-p=0

    @param p const Polynomial *
    @param v const Var *

    @return true if p is a valid extension polynomial
*/
static bool check_for_valid_extension_poly(
  const Polynomial * p, const Var * v) {
  if (v->get_count() > 1) return 0;

  Polynomial * mult = multiply_poly(p, p);
  bool zero = equal_polynomials(mult, p);
  delete(mult);
  return zero;
}

/***************************************************************************/
/**
    Parses an extension rule with starting index 'index'

    @param index unsigned

*/
static void parse_extension_rule(unsigned index) {
  if (find_inference_index(index))
    parse_error("index %i already exists", index);

  unsigned line = lineno_at_start_of_last_token;
  Polynomial * p1 = parse_polynomial();

  if (!check_for_valid_extension_var(p1)) {
    fflush(stdout);
    fprintf(stderr, "*** 'pacheck' error in EXTENSION_RULE rule with index %i ",
      index);
    fprintf(stderr, " in '%s' line %i: extension variable is not valid",
      parse_file_name, line);
    fputc('\n', stderr);
    fflush(stderr);
    exit(1);
  }
  const Var * ext = p1->get_lt()->get_var();

  assert(is_comma_token());

  Polynomial * p2 = parse_polynomial(0);
  if (!check_for_valid_extension_poly(p2, ext)) {
    fflush(stdout);
    fprintf(stderr, "*** 'pacheck' error in EXTENSION_RULE rule with index %i ",
      index);
    fprintf(stderr, " in '%s' line %i is not a valid extension polynomial",
      parse_file_name, line);
    fputc('\n', stderr);
    fflush(stderr);
    exit(1);
  }

  if (!is_semicolon_token()) parse_error("unexpected %s token", get_token());

  Polynomial * p3 = negate_poly(p1);
  Polynomial * q = add_poly(p2, p3);
  delete(p1);
  delete(p2);
  delete(p3);

  new_inference(index, q);
  extension_inferences++;
}

/*------------------------------------------------------------------------*/

/// used to collect the factors of each slice for PAC proofs
static std::vector<Polynomial*> factor_array;

/*-------------------------------------------------------------------------*/
/**
    Adds up products of the linear combination

    @return Polynomial *

*/
static Polynomial * add_up_products() {
  while (factor_array.size() > 1) {
    Polynomial * p = factor_array.back();
    factor_array.pop_back();
    Polynomial * q = factor_array.back();
    factor_array.pop_back();

    Polynomial * add = add_poly(p, q);
    delete(p);
    delete(q);
    factor_array.push_back(add);
  }
  Polynomial * res = factor_array.back();
  factor_array.pop_back();
  return res;
}
/*------------------------------------------------------------------------*/
/**
    Merges the products in tree, depth first

*/
static void merge_products() {
  unsigned i = factor_array.size();
  if (i == 1) return;
  Polynomial * p = factor_array[i-1];
  Polynomial * q = factor_array[i-2];
  int p_level = p->get_level();

  if (p_level == q->get_level()) {
    Polynomial * add = add_poly(p, q);
    delete(p);
    delete(q);
    add->set_level(p_level+1);

    factor_array.pop_back();
    factor_array.pop_back();
    factor_array.push_back(add);
    merge_products();
  }
}

/*------------------------------------------------------------------------*/
/**
    Parses a linear combination rule with starting index 'index'

    @param index unsigned

*/
static void parse_lin_combination_rule(int index) {
  if (find_inference_index(index))
    parse_error("index %i already exists", index);

  unsigned rule_line = lineno_at_start_of_last_token;
  unsigned p_index;
  const Inference * i0;
  Polynomial * tmp, * conclusion = 0;

  next_token();
  while (!is_comma_token()) {
    p_index = parse_index();
    i0 = find_inference_index(p_index);
    if (!i0) polynomial_not_found(index, p_index, rule_line);

    next_token();
    if (is_multiply_token()) {
      multiplication_operations++;
      next_token();
      if (!is_open_parenthesis_token()) parse_error("expected '('");

      Polynomial * p = parse_polynomial(0);
      tmp = multiply_poly(i0->get_conclusion(), p);
      delete(p);

      assert(is_close_parenthesis_token());
      next_token();
    } else {
      tmp = i0->get_conclusion()->copy();
    }

    factor_array.push_back(tmp);
    merge_products();


    if (is_plus_token()) {
      addition_operations++;
      next_token();
    } else if (!is_comma_token()) {
      parse_error("unexpected '%s'", get_token());
    }
  }
  conclusion = add_up_products();

  unsigned p2_line = lineno_at_start_of_last_token;
  Polynomial *p2 = parse_polynomial();
  assert(is_semicolon_token());

  if (!equal_polynomials(p2, conclusion))
    polynomials_do_not_match(index, p2, conclusion, rule_line, p2_line);
  delete(p2);

  if (check_target && equal_polynomials(conclusion, target)) {
    target_polynomial_inferences = 1;
  }

  new_inference(index, conclusion);
  lin_comb_inferences++;
}

/***************************************************************************/
/**
    Parses the axioms from file_name

    @param file_name const char *

*/

static void parse_original_polynomials(const char * file_name) {
  init_parsing(file_name);
  msg("reading original polynomials from '%s'", parse_file_name);
  int original = 0;
  while (!following_token_is_EOF()) {
    unsigned line = lineno_at_start_of_last_token;
    unsigned index = parse_index();

    if (find_inference_index(index))
      parse_error("error in line %i index %i already exists", line, index);

    Polynomial * p = parse_polynomial();
    if (!is_semicolon_token())
      parse_error("error in line %i unexpected %s token", line, get_token());

    new_inference(index, p);
    original_inferences++;

    if (check_target && equal_polynomials(p, target)) {
      fprintf(stdout, "\n");
      msg("WARNING: target polynomial is given as original polynomial.");
      msg("Proof rules are obsolet, but will be checked anyway!\n");
      target_polynomial_inferences = 1;
    }
    original++;
  }
  msg("found %i original polynomials in '%s'", original, parse_file_name);
  reset_parsing();
}

/*------------------------------------------------------------------------*/
/**
    Parses the proof from file_name

    @param file_name const char *

*/
static void parse_and_check_proof_rules(const char * file_name) {
  init_parsing(file_name);
  msg("reading polynomial algebraic calculus proof from '%s'",
    parse_file_name);
  unsigned checked = 0, extensions = 0;

  while (!following_token_is_EOF()) {
    int index = parse_index();
    next_token();

    if (is_delete_token()) {
      deletion_inferences++;
      if (delete_mode) delete_inference_by_index(index);
      next_token();
      if (!is_semicolon_token())
        parse_error("unexpected %s token", get_token());
    } else if (is_extension_token()) {
      parse_extension_rule(index);
      extensions++;
      num_inference_rules++;
      checked++;
    } else if (is_lin_combi_token()) {
      parse_lin_combination_rule(index);
      num_inference_rules++;
      checked++;
      if (verbose && checked % 1000 == 0)
           msg("found and checked %6" PRIu64 " inferences so far", checked);
    } else {
      parse_error("expected operator 'd', '=' or '%%'");
    }
  }

  msg("found and checked %" PRIu64 " inferences in '%s'",
    checked, parse_file_name);
  reset_parsing();
}

/*------------------------------------------------------------------------*/

void parse_target_polynomial(const char * file_name) {
  init_parsing(file_name);
  msg("reading target polynomial from '%s'", parse_file_name);
  target = parse_polynomial();
  assert(is_semicolon_token());
  if (!following_token_is_EOF()) die("unexpected %s token", get_token());
  reset_parsing();
}

/*------------------------------------------------------------------------*/

void parse_and_check_proof(const char * polys_file_name,
                           const char * rule_file_name) {
  init_mpz();
  parse_original_polynomials(polys_file_name);
  parse_and_check_proof_rules(rule_file_name);
}

/*------------------------------------------------------------------------*/

void checker_statistics() {
  print_statistics(original_inferences, extension_inferences,
    lin_comb_inferences, deletion_inferences, num_inference_rules,
    addition_operations,
    multiplication_operations);
}
/*------------------------------------------------------------------------*/

void reset() {
  clear_mpz();
  delete(target);
  delete_inferences();

  deallocate_mstack();
  deallocate_var_list();
  deallocate_terms();
  deallocate_variables();
  deallocate_buffer();
}
