/*------------------------------------------------------------------------*/
/*! \file polynomial.cpp
    \brief contains arithmetic operations for polynomials

  Part of Pacheck 2.0 : PAC proof checker.
  Copyright(C) 2020 Daniela Kaufmann, Johannes Kepler University Linz
*/
/*------------------------------------------------------------------------*/
#include <algorithm>
#include "polynomial.h"
/*------------------------------------------------------------------------*/

Polynomial::Polynomial(Monomial * m, Polynomial * p):
  mon(m), rest(p) {
    if(m && !p) siz = 1;
    else if (m && p) siz = p->siz+1;
  }


unsigned Polynomial::size() const {
  return siz;
}


unsigned Polynomial::min_term_size() const {
    unsigned len = INT_MAX;
    const Polynomial * res = this;

    while(res){
      Monomial * m = res->get_lm();
      unsigned tlen = 0;
      if (m->get_term()) tlen = m->get_term_size();
      if (tlen < len) len = tlen;
      res = res->rest;
    }
    return len;
}

/*------------------------------------------------------------------------*/

unsigned Polynomial::degree() const {
    unsigned len = 0;
    const Polynomial * res = this;

    while(res){
      Monomial * m = res->get_lm();
      unsigned tlen = 0;
      if (m && m->get_term()) tlen = m->get_term_size();
      if (tlen > len) len = tlen;
      res = res->rest;
    }
    return len;
}

/*------------------------------------------------------------------------*/

bool Polynomial::is_constant_zero_poly() const {
  const Polynomial * res = this;
  if(!res) return 1;
  if(!res->get_lm() && !res->get_rest()) return 1;
  return 0;
}

/*------------------------------------------------------------------------*/

bool Polynomial::is_constant_one_poly() const {
  const Polynomial * res = this;

  if (res->size() != 1) return 0;

  Monomial * m = res->get_lm();
  if (m->get_term()) return 0;
  if (mpz_cmp_si(m->coeff, 1) != 0) return 0;

  return 1;
}

/*------------------------------------------------------------------------*/

Polynomial * Polynomial::copy() const {
  const Polynomial * res = this;
  if(res->is_constant_zero_poly()) return zero_poly();
  while(res){
    push_mstack(res->get_lm()->copy());
    res = res->get_rest();
  }
  Polynomial * out = build_poly(0);
  return out;
}

/*------------------------------------------------------------------------*/

void Polynomial::print(FILE * file, bool end) const {
  const Polynomial * res = this;

  if (!res) {fputc ('0', file);
  } else if(!res->rest) {
    Monomial * m = res->get_lm();
    if(m) m->print(file, 1);
    else fputc ('0', file);
  } else {
    Monomial * m = res->get_lm();
    if(m) m->print(file, 1);

    for (Polynomial * q = res->rest; q; q = q->rest) {
      if(q->get_lm()) q->get_lm()->print(file, 0);
    }
  }
  if (end) fputs(";\n", file);
}


/*------------------------------------------------------------------------*/

Polynomial::~Polynomial() {
  Polynomial * res = this;
  Polynomial * rest = res->get_rest();
  deallocate_monomial(res->get_lm());
  delete(rest);
}


/*------------------------------------------------------------------------*/

// Local variables
static size_t size_mstack;  ///< size of mstack
static size_t num_mstack = 0;  ///< number of elements in mstack
static Monomial ** mstack;  ///< Monomial** used for building poly
/*------------------------------------------------------------------------*/
/**
    Enlarges the allocated size of mstack
*/
static void enlarge_mstack() {
  size_t new_size_mstack = size_mstack ? 2*size_mstack : 1;

  Monomial** newArr = new Monomial*[new_size_mstack];
  memcpy( newArr, mstack, size_mstack * sizeof(Monomial*));
  delete [] mstack;
  mstack = newArr;
  size_mstack = new_size_mstack;
}

/*------------------------------------------------------------------------*/
/**
    Sets the number of elements in the stack to 0
*/
static void clear_mstack() { num_mstack = 0; }

/*------------------------------------------------------------------------*/

void deallocate_mstack() { delete[](mstack); }

/*------------------------------------------------------------------------*/

/**
    Pushes a monomial to the end of the stack

    @param t monomial to be added to the mstack
*/
void push_mstack(Monomial *m) {
  if (size_mstack == num_mstack) enlarge_mstack();

  assert(m);
  if (mpz_sgn(m->coeff) == 0) {
    deallocate_monomial(m);
    return;
  }

  mstack[num_mstack++] = m;
}

/*------------------------------------------------------------------------*/
static int cmp_monomials_for_qsort (const void * p, const void * q) {
  Monomial * a = *(Monomial **) p;
  Monomial * b = *(Monomial **) q;
  if(a->get_term() && b->get_term()) return -1*(a->get_term()->cmp(b->get_term()));
  else if (a->get_term()) return -1;
  else return 1;
}

/*------------------------------------------------------------------------*/

static void sort_monomials () {
  qsort (mstack, num_mstack, sizeof *mstack, cmp_monomials_for_qsort);
}

/*------------------------------------------------------------------------*/

static void merge_monomials () {
  Monomial * a = 0;
  size_t i = 0;

  mpz_t tmp_gmp;
  mpz_init(tmp_gmp);

  for (size_t j = 0; j < num_mstack; j++) {
    Monomial * b = mstack[j];
    if (mpz_sgn(b->coeff) == 0) {
      deallocate_monomial(b);
      i--;
    } else if (a && a->get_term() == b->get_term()) {
      mpz_add(tmp_gmp, a->coeff, b->coeff);
      deallocate_monomial(b);
      if (mpz_sgn(tmp_gmp) != 0) {
        Monomial * c = new Monomial(tmp_gmp, a->get_term()->copy());
        deallocate_monomial(a);
        mstack[i-1] = c;
        a = c;
      } else {
        deallocate_monomial(a);
        i--;
        if (i > 0) a = mstack[i-1];
        else
          a = 0;
      }
    } else {
      mstack[i++] = b;
      a = b;
    }
  }

  mpz_clear(tmp_gmp);
  num_mstack = i;
}


/*------------------------------------------------------------------------*/

Polynomial * build_poly(bool need_sorting) {
  if (need_sorting) {
    sort_monomials();
    merge_monomials();
  }
  Polynomial * res = 0;
  int i = num_mstack;
  if(!i) res = new Polynomial(0, 0);
  while (i > 0) {
    res = new Polynomial(mstack[--i], res);
  }


  clear_mstack();
  return res;
}


/*------------------------------------------------------------------------*/

bool equal_polynomials(const Polynomial * p1, const Polynomial * p2) {
  assert(p1);
  assert(p2);
  if (p1->is_constant_zero_poly() && p2->is_constant_zero_poly()) return 1;

  const Polynomial * tmp1 = p1;
  const Polynomial * tmp2 = p2;

  Monomial * m1, * m2;

  while (tmp1 && tmp2) {
    m1 = tmp1->get_lm();
    m2 = tmp2->get_lm();
    if (m1 == m2) return 1;
    if (!m1) return 0;
    if (!m2) return 0;
    if (m1->get_term() != m2->get_term()) return 0;
    if (mpz_cmp(m1->coeff, m2->coeff) != 0) return 0;

    tmp1 = tmp1->get_rest();
    tmp2 = tmp2->get_rest();

  }

  if (tmp1 || tmp2 ) return 0;
  return 1;
}

/*------------------------------------------------------------------------*/

Polynomial * add_poly(const Polynomial * p1, const Polynomial * p2) {

  if(p1->is_constant_zero_poly() && p2->is_constant_zero_poly()) return zero_poly();
  if(p1->is_constant_zero_poly()) return p2->copy();
  if(p2->is_constant_zero_poly()) return p1->copy();

  assert(p1);
  assert(p2);

  mpz_t tmp_gmp;
  mpz_init(tmp_gmp);

  const Polynomial * tmp1 = p1;
  const Polynomial * tmp2 = p2;
  Monomial * m1, *m2;

  while (tmp1 && tmp2) {
    m1 = tmp1->get_lm();
    m2 = tmp2->get_lm();

    if (!m1->get_term() || !m2->get_term()) {
      if (!m1->get_term() && !m2->get_term()) {
        mpz_add(tmp_gmp, m1->coeff, m2->coeff);
        if (mpz_sgn(tmp_gmp) != 0) {
          Monomial * m = new Monomial(tmp_gmp, 0);
          push_mstack(m);
        }
        tmp1 = tmp1->get_rest();
        tmp2 = tmp2->get_rest();


      } else if (!m1->get_term()) {
        push_mstack(m2->copy());
        tmp2 = tmp2->get_rest();
      } else {
        push_mstack(m1->copy());
        tmp1 = tmp1->get_rest();
      }
    } else {
      if (m1->get_term() == m2->get_term()) {
        mpz_add(tmp_gmp, m1->coeff, m2->coeff);
        if (mpz_sgn(tmp_gmp) != 0) {
          Monomial * m = new Monomial(tmp_gmp, m1->get_term_copy());
          push_mstack(m);
        }
        tmp1 = tmp1->get_rest();
        tmp2 = tmp2->get_rest();
      } else {
        if (m1->get_term()->cmp(m2->get_term()) > 0) {
          push_mstack(m1->copy());
          tmp1 = tmp1->get_rest();
        } else {
          push_mstack(m2->copy());
          tmp2 = tmp2->get_rest();
        }
      }
    }
  }

  mpz_clear(tmp_gmp);

  while (tmp1) {
    m1 = tmp1->get_lm();
    push_mstack(m1->copy());
    tmp1 = tmp1->get_rest();

  }
  while (tmp2) {
    m2 = tmp2->get_lm();
    push_mstack(m2->copy());
    tmp2 = tmp2->get_rest();

  }

  Polynomial * p = build_poly(0);
  return p;
}


/*------------------------------------------------------------------------*/

Polynomial * multiply_poly(const Polynomial * p1, const Polynomial * p2) {

  if(p1->is_constant_zero_poly() || p2->is_constant_zero_poly()) return zero_poly();

  Term * t;
  const Polynomial * tmp1 = p1;
  const Polynomial * tmp2 = p2;
  const Monomial *m1, *m2;

  mpz_t tmp_gmp;
  mpz_init(tmp_gmp);

  while (tmp1) {
    m1 = tmp1->get_lm();
    tmp2 = p2;
    while (tmp2) {
      m2 = tmp2->get_lm();
      mpz_mul(tmp_gmp, m1->coeff, m2->coeff);

      if (m1->get_term() && m2->get_term())
        t = multiply_term(m1->get_term(), m2->get_term());
      else if (m2->get_term()) t = m2->get_term_copy();
      else if (m1->get_term()) t = m1->get_term_copy();
      else
        t = 0;
      push_mstack(new Monomial(tmp_gmp, t));
      tmp2 = tmp2->get_rest();
    }
    tmp1 = tmp1->get_rest();
  }

  mpz_clear(tmp_gmp);

  Polynomial * p = build_poly(1);
  return p;
}

/*------------------------------------------------------------------------*/

Polynomial * negate_poly(const Polynomial *p1) {

  if(p1->is_constant_zero_poly()) return zero_poly();

  mpz_t tmp_gmp;
  mpz_init(tmp_gmp);

  const Polynomial * tmp1 = p1;

  while(tmp1){
    Monomial * m = tmp1->get_lm();
    mpz_neg(tmp_gmp, m->coeff);
    if (m->get_term())
      push_mstack(new Monomial(tmp_gmp, m->get_term_copy()));
    else
      push_mstack(new Monomial(tmp_gmp, 0));

    tmp1 = tmp1->get_rest();
  }

  mpz_clear(tmp_gmp);
  Polynomial * tmp = build_poly(0);
  return tmp;
}

Polynomial * zero_poly() {
  Polynomial * res = new Polynomial(0, 0);
  return res;
}

/*------------------------------------------------------------------------*/
