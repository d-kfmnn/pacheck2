/*------------------------------------------------------------------------*/
/*! \file monomial.cpp
    \brief contains the class Monomial and further functions to
    manipulate monomials

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#include "monomial.h"
/*------------------------------------------------------------------------*/

Monomial::Monomial(mpz_t _c, Term * _t): ref(1) {
  mpz_init_set(coeff, _c);
  if (mpz_sgn(_c) == 0) term = 0;
  else  term = _t;
}

/*------------------------------------------------------------------------*/

Monomial * Monomial::copy() {
  Monomial * m = this;
  if (!m) return 0;
  assert(ref > 0);

  ++ref;
  assert(ref);
  return this;
}

/*------------------------------------------------------------------------*/

Monomial::~Monomial() {
  assert(ref == 0);

  mpz_clear(coeff);
  deallocate_term(term);
}

/*------------------------------------------------------------------------*/

void Monomial::print(FILE * file, bool lm) const {
  int sign = mpz_sgn(coeff);
  if (!sign) return;
  else if (!lm && sign > 0) fputc_unlocked('+', file);

  if (term) {
    if (mpz_cmp_si(coeff, -1) == 0) { fputc_unlocked('-', file);
    } else if (mpz_cmp_si(coeff, 1) != 0) {
      mpz_out_str(file, 10, coeff);
      fputc_unlocked('*', file);
    }

    term->print(file);
  }
  else  mpz_out_str(file, 10, coeff);
}

/*------------------------------------------------------------------------*/

Monomial * multiply_monomial(const Monomial * m1, const Monomial *m2) {
  assert(m1);
  assert(m2);

  mpz_t tmp_gmp;
  mpz_init(tmp_gmp);

  mpz_mul(tmp_gmp, m1->coeff, m2->coeff);

  Term * t;
  if (m1->get_term() && m2->get_term())
    t = multiply_term(m1->get_term(), m2->get_term());
  else if (m2->get_term()) t = m2->get_term_copy();
  else if (m1->get_term()) t = m1->get_term_copy();
  else
    t = 0;

  Monomial * mon = new Monomial(tmp_gmp, t);
  mpz_clear(tmp_gmp);
  return mon;
}

/*------------------------------------------------------------------------*/

void deallocate_monomial(Monomial * m) {
  if(!m) return;

  assert(m->get_ref() > 0);
  if (m->dec_ref() > 0) return;

  delete(m);
}



/*------------------------------------------------------------------------*/
