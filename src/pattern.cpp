/*------------------------------------------------------------------------*/
/*! \file pattern.cpp
    \brief core functions for pattern matching

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2025 Daniela Kaufmann, TU Wien
*/
/*------------------------------------------------------------------------*/
#include "pattern.h"

#include <map>

unsigned new_patterns_count = 0;
unsigned apply_patterns_count = 0;
/*------------------------------------------------------------------------*/
struct Pattern {
  std::vector<Polynomial*> inp;
  std::vector<Polynomial*> outp;
};

std::map<size_t, Pattern*> patterns;
std::map<int, Polynomial*> temporary_inferences;
std::map<const Var*, const Var*> temporary_var_matching;
/*------------------------------------------------------------------------*/
/**
    Prints an error message that polynomials 'actual' and 'expected' do not match

    @param index unsigned
    @param actual const Polynomial *
    @param expected const Polynomial *
    @param rule_line unsigned
    @param polynomial_line unsigned
*/
void polynomials_do_not_match(
    unsigned index, const Polynomial* actual, const Polynomial* expected,
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
  fflush(stderr);
  exit(1);
}

static bool is_input_word(const std::string& word) {
  if (word.size() < 3) return false;
  if (word.substr(0, 2) != "in") return false;
  for (size_t i = 2; i < word.size(); ++i) {
    if (!isdigit(word[i])) return false;
  }
  return true;
}

static bool is_matching_var_word(const std::string& word) {
  if (word.size() < 2) return false;
  if (word.substr(0, 1) != "v") return false;
  for (size_t i = 2; i < word.size(); ++i) {
    if (!isdigit(word[i])) return false;
  }
  return true;
}

static bool is_index_word(const std::string& word) {
  for (size_t i = 0; i < word.size(); ++i) {
    if (!isdigit(word[i])) return false;
  }
  return true;
}

static bool is_output_word(const std::string& word) {
  if (word.size() < 4) return false;
  if (word.substr(0, 3) != "out") return false;
  for (size_t i = 3; i < word.size(); ++i) {
    if (!isdigit(word[i])) return false;
  }
  return true;
}

/*--------------------------------------------------------------*/
static Polynomial* rematch(Polynomial* p, std::map<const Var*, const Var*> matching) {
  if (!p) return 0;

  Polynomial* tmp = p;
  Monomial* m;

  while (tmp) {
    m = tmp->get_lm();
    if (!m->get_term())
      push_mstack(m->copy());
    else {
      Term* t = m->get_term();
      while (t) {
        push_var_list(matching[t->get_var()]);
        t = t->get_rest();
      }
      Term* t_match = build_term_from_list();
      Monomial* m_match = new Monomial(m->coeff, t_match);
      push_mstack(m_match);
    }
    tmp = tmp->get_rest();
  }

  return build_poly(1);
}

/*--------------------------------------------------------------*/
static void parse_pattern_lin_combination_rule(int index) {
  if (temporary_inferences.find(index) != temporary_inferences.end())
    parse_error("temporary inference %li already exists", index);

  unsigned rule_line = lineno_at_start_of_last_token;
  unsigned p_index;
  const Polynomial* i0;
  const Polynomial *tmp, *conclusion = 0;
  std::vector<const Polynomial*> factor_array;

  next_token();
  while (!is_comma_token()) {
    p_index = parse_index();

    if (temporary_inferences.find(p_index) == temporary_inferences.end())
      parse_error("index not found");

    i0 = temporary_inferences[p_index];

    next_token();
    if (is_multiply_token()) {
      next_token();
      if (!is_open_parenthesis_token()) parse_error("expected '('");

      Polynomial* p = parse_polynomial(0);

      tmp = multiply_poly(i0, p);
      delete (p);

      assert(is_close_parenthesis_token());
      next_token();
    } else {
      tmp = i0->copy();
    }

    Polynomial* add_tmp = add_poly(conclusion, tmp);
    delete (conclusion);
    conclusion = add_tmp;
    delete (tmp);

    if (is_plus_token()) {
      next_token();
    } else if (!is_comma_token()) {
      parse_error("unexpected '%s'", get_token());
    }
  }

  unsigned p2_line = lineno_at_start_of_last_token;
  Polynomial* p2 = parse_polynomial(0);
  assert(is_semicolon_token());

  if (!equal_polynomials(p2, conclusion))
    polynomials_do_not_match(index, p2, conclusion, rule_line, p2_line);
  delete (conclusion);

  temporary_inferences.insert({index, p2});
}
/*--------------------------------------------------------------*/
static std::vector<Polynomial*> parse_new_pattern_input(std::vector<Polynomial*> inputs) {
  next_token();
  size_t index = parse_index();
  if (temporary_inferences.find(index) != temporary_inferences.end()) parse_error("temporary inference %li already exists", index);

  Polynomial* p = parse_polynomial(1);

  if (!is_semicolon_token()) {
    parse_error("expected a semicolon");
  }

  temporary_inferences.insert({index, p});
  inputs.push_back(p);
  return inputs;
}
/*--------------------------------------------------------------*/
static void parse_new_pattern_steps(std::string word) {
  int index = atoi(word.c_str());

  next_token();
  if (!is_lin_combi_token()) {
    parse_error("expected a linear combination rule");
  }
  parse_pattern_lin_combination_rule(index);
}
/*--------------------------------------------------------------*/
static std::vector<Polynomial*> parse_new_pattern_output(std::vector<Polynomial*> outputs) {
  next_token();
  size_t index = parse_index();

  if (temporary_inferences.find(index) == temporary_inferences.end())
    parse_error("temporary inference %li does not exist", index);

  next_token();
  if (!is_semicolon_token()) {
    parse_error("expected a semicolon");
  }
  Polynomial* p = temporary_inferences[index];

  outputs.push_back(p);
  return outputs;
}
/*--------------------------------------------------------------*/
static void parse_new_pattern(size_t index) {
  if (patterns.find(index) != patterns.end()) parse_error("pattern with index %lu already exists", index);
  temporary_inferences.clear();

  next_token();
  std::string word = parse_word();

  std::vector<Polynomial*> inputs;
  std::vector<Polynomial*> outputs;

  while (!is_curly_close_token()) {
    // read input poly of pattern
    if (is_input_word(word)) {
      inputs = parse_new_pattern_input(inputs);
      next_token();
      word = parse_word();
    }

    // read proof steps
    else if (is_index_word(word)) {
      parse_new_pattern_steps(word);
      next_token();
      word = parse_word();
    }

    // read outputs
    else if (is_output_word(word)) {
      outputs = parse_new_pattern_output(outputs);
      next_token();
      word = parse_word();
    }

    else
      parse_error("expected a closing curly brace");
  }

  Pattern* pat = new Pattern();
  pat->inp = inputs;
  pat->outp = outputs;

  patterns.insert({index, pat});
  new_patterns_count++;

  next_token();
}

/*------------------------------------------------------------------------*/
static void check_pattern_parse_matching(std::string word) {
  const Var* pattern_var = new_variable(word.c_str(), 0);
  next_token();
  const Var* apply_var = parse_variable(1);

  temporary_var_matching.insert({pattern_var, apply_var});

  if (!is_semicolon_token()) {
    parse_error("expected a semicolon");
  }
}
/*------------------------------------------------------------------------*/
static void check_pattern_parse_input(Pattern* pattern, int i) {
  next_token();
  size_t index = parse_index();

  const Inference* i0 = find_inference_index(index);
  if (!i0) parse_error("error in pattern, inference %li not found", index);

  const Polynomial* p0 = i0->get_conclusion();

  Polynomial* pattern_p0 = pattern->inp[i];

  Polynomial* rematched_pattern_p0 = rematch(pattern_p0, temporary_var_matching);

  if (!equal_polynomials(p0, rematched_pattern_p0))
    polynomials_do_not_match(index, p0, rematched_pattern_p0, 0, 0);

  delete (rematched_pattern_p0);

  next_token();
  if (!is_semicolon_token()) {
    parse_error("expected a semicolon");
  }
}
/*------------------------------------------------------------------------*/
static void check_pattern_parse_output(Pattern* pattern, int i) {
  next_token();
  size_t index = parse_index();

  Polynomial* p = parse_polynomial();

  Polynomial* pattern_p0 = pattern->outp[i];

  Polynomial* rematched_pattern_p0 = rematch(pattern_p0, temporary_var_matching);

  if (!equal_polynomials(p, rematched_pattern_p0))
    polynomials_do_not_match(index, p, rematched_pattern_p0, 0, 0);

  new_inference(index, p);

  if (!is_semicolon_token()) {
    parse_error("expected a semicolon");
  }
}
/*------------------------------------------------------------------------*/
static void check_pattern(size_t index) {
  if (patterns.find(index) == patterns.end())
    parse_error("pattern with index %lu not found", index);

  Pattern* p = patterns[index];

  temporary_var_matching.clear();

  next_token();
  std::string word = parse_word();

  // read matching var
  while (is_matching_var_word(word)) {
    check_pattern_parse_matching(word);
    next_token();
    word = parse_word();
  }

  // read input polynomials
  int i = 0;
  while (is_input_word(word)) {
    check_pattern_parse_input(p, i++);
    next_token();
    word = parse_word();
  }

  // read output polynomials
  i = 0;
  while (is_output_word(word)) {
    check_pattern_parse_output(p, i++);
    next_token();
    word = parse_word();
  }

  apply_patterns_count++;
  if (!is_curly_close_token()) parse_error("expected a closing curly brace");
  next_token();
}

/*------------------------------------------------------------------------*/
void delete_pattern(size_t index) {
  patterns.erase(index);
}

/*------------------------------------------------------------------------*/
void parse_pattern() {
  std::string word = parse_word();
  if (word.empty()) {
    parse_error("expected a pattern but received an empty word");
  }

  if (word != "pattern_new" && word != "pattern_apply" && word != "pattern_delete") {
    parse_error("expected a pattern");
  }
  next_token();

  size_t index = parse_index();
  next_token();

  if (word == "pattern_delete") {
    delete_pattern(index);
  } else {
    if (!is_curly_open_token()) {
      parse_error("expected an open curly brace");
    }

    if (word == "pattern_new")
      parse_new_pattern(index);
    else
      check_pattern(index);
  }

  if (!is_semicolon_token()) {
    parse_error("expected a semicolon");
  }
}