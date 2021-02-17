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

unsigned Polynomial::min_term_size() const {
    unsigned len = INT_MAX;

    for (std::deque<Monomial *>::const_iterator it = mon_begin();
        it != mon_end(); ++it) {
      Monomial * m = *it;

      unsigned tlen = 0;
      if (m->get_term()) tlen=m->get_term_size();
      if (tlen < len) len = tlen;
    }
    return len;
}

/*------------------------------------------------------------------------*/

unsigned Polynomial::degree() const {
    unsigned len = 0;

    for (std::deque<Monomial *>::const_iterator it = mon_begin();
        it != mon_end(); ++it) {
      Monomial * m = *it;
      unsigned tlen = 0;
      if (m->get_term()) tlen = m->get_term_size();
      if (tlen > len) len = tlen;
    }
    return len;
}

/*------------------------------------------------------------------------*/

bool Polynomial::is_constant_zero_poly() const {
  return mon.empty();
}

/*------------------------------------------------------------------------*/

bool Polynomial::is_constant_one_poly() const {
  if (mon.size() != 1) return 0;

  Monomial * m = mon.front();
  if (m->get_term()) return 0;
  if (mpz_cmp_si(m->coeff, 1) != 0) return 0;

  return 1;
}

/*------------------------------------------------------------------------*/

Polynomial * Polynomial::copy() const {
  Polynomial * out = new Polynomial();
  for (std::deque<Monomial *>::const_iterator it = mon_begin();
      it != mon_end(); ++it) {
    Monomial * m = *it;
    out->mon.push_back(m->copy());
  }
  return out;
}

/*------------------------------------------------------------------------*/

void Polynomial::print(FILE * file, bool end) const {
  if (mon.empty()) { fputs_unlocked("0", file);
  } else {
    for (std::deque<Monomial *>::const_iterator it = mon_begin();
         it != mon_end(); ++it) {
      Monomial * m = *it;
      if (it == mon_begin()) m->print(file, 1);
      else
        m->print(file, 0);
    }
  }
  if (end) fputs(";\n", file);
}

/*------------------------------------------------------------------------*/

Polynomial::~Polynomial() {
  for (std::deque<Monomial *>::const_iterator it = mon_begin();
      it != mon_end(); ++it)

    deallocate_monomial(*it);
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

  for (size_t j = 0; j < num_mstack; j++) {
    Monomial * b = mstack[j];
    if (mpz_sgn(b->coeff) == 0) {
      deallocate_monomial(b);
      i--;
    } else if (a && a->get_term() == b->get_term()) {
      mpz_add(dummy_gmp, a->coeff, b->coeff);
      deallocate_monomial(b);
      if (mpz_sgn(dummy_gmp) != 0) {
        Monomial * c = new Monomial(dummy_gmp, a->get_term()->copy());
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
  num_mstack = i;
}


/*------------------------------------------------------------------------*/

Polynomial * build_poly(bool need_sorting) {
  if (need_sorting) {
    sort_monomials();
    merge_monomials();
  }
  Polynomial * res = new Polynomial();
  for (size_t i = 0; i< num_mstack; i++) {
    res->mon_push_back(mstack[i]);
  }

  clear_mstack();
  return res;
}

/*------------------------------------------------------------------------*/

bool equal_polynomials(const Polynomial * p1, const Polynomial * p2) {
  assert(p1);
  assert(p2);


  std::deque<Monomial*>::const_iterator it1 = p1->mon_begin()++;
  std::deque<Monomial*>::const_iterator it2 = p2->mon_begin()++;
  Monomial * m1 = *it1;
  Monomial * m2 = *it2;

  while (it1 != p1->mon_end() && it2 != p2->mon_end()) {
    if (m1->get_term() != m2->get_term()) {
      return 0;
    }
    if (mpz_cmp(m1->coeff, m2->coeff) != 0) {
      return 0;
    }
    ++it1;
    ++it2;
    m1 = *it1;
    m2 = *it2;
  }

  if (it1 != p1->mon_end()) return 0;
  if (it2 != p2->mon_end()) return 0;
  return 1;
}

/*------------------------------------------------------------------------*/

Polynomial * add_poly(const Polynomial * p1, const Polynomial * p2) {
  assert(p1);
  assert(p2);

  std::deque<Monomial*>::const_iterator it1 = p1->mon_begin()++;
  std::deque<Monomial*>::const_iterator it2 = p2->mon_begin()++;
  Monomial * m1 = *it1;
  Monomial * m2 = *it2;
  while (it1 != p1->mon_end() && it2 != p2->mon_end()) {
    if (!m1->get_term() || !m2->get_term()) {
      if (!m1->get_term() && !m2->get_term()) {
        mpz_add(dummy_gmp, m1->coeff, m2->coeff);
        if (mpz_sgn(dummy_gmp) != 0) {
          Monomial * m = new Monomial(dummy_gmp, 0);
          push_mstack(m);
        }
        ++it1;
        ++it2;
        m1 = *it1;
        m2 = *it2;
      } else if (!m1->get_term()) {
        push_mstack(m2->copy());
        ++it2;
        m2 = *it2;
      } else {
        push_mstack(m1->copy());
        ++it1;
        m1 = *it1;
      }
    } else {
      if (m1->get_term() == m2->get_term()) {
        mpz_add(dummy_gmp, m1->coeff, m2->coeff);
        if (mpz_sgn(dummy_gmp) != 0) {
          Monomial * m = new Monomial(dummy_gmp, m1->get_term_copy());
          push_mstack(m);
        }
        ++it1;
        ++it2;
        m1 = *it1;
        m2 = *it2;

      } else {
        if (m1->get_term()->cmp(m2->get_term()) > 0) {
          push_mstack(m1->copy());
          ++it1;
          m1 = *it1;
        } else {
          push_mstack(m2->copy());
          ++it2;
          m2 = *it2;
        }
      }
    }
  }

  while (it1 != p1->mon_end()) {
    push_mstack(m1->copy());
    ++it1;
    m1 = *it1;
  }
  while (it2 != p2->mon_end()) {
    push_mstack(m2->copy());
    ++it2;
    m2 = *it2;
  }

  Polynomial * p = build_poly(0);
  return p;
}


/*------------------------------------------------------------------------*/

Polynomial * multiply_poly(const Polynomial * p1, const Polynomial * p2) {
  Term * t;
  std::deque<Monomial*>::const_iterator it1 = p1->mon_begin();
  std::deque<Monomial*>::const_iterator it2;
  const Monomial *m1, *m2;

  while (it1 != p1->mon_end()) {
    m1 = *it1;
    it2 = p2->mon_begin();
    while (it2 != p2->mon_end()) {
      m2 = *it2;
      mpz_mul(dummy_gmp, m1->coeff, m2->coeff);

      if (m1->get_term() && m2->get_term())
        t = multiply_term(m1->get_term(), m2->get_term());
      else if (m2->get_term()) t = m2->get_term_copy();
      else if (m1->get_term()) t = m1->get_term_copy();
      else
        t = 0;
      push_mstack(new Monomial(dummy_gmp, t));
      ++it2;
    }
    ++it1;
  }

  Polynomial * p = build_poly(1);
  return p;
}

/*------------------------------------------------------------------------*/

Polynomial * negate_poly(const Polynomial *p1) {
  for (std::deque<Monomial*>::const_iterator it = p1->mon_begin();
      it != p1->mon_end(); ++it) {
    Monomial * m = *it;
    mpz_neg(dummy_gmp, m->coeff);
    if (m->get_term())
      push_mstack(new Monomial(dummy_gmp, m->get_term_copy()));
    else
      push_mstack(new Monomial(dummy_gmp, 0));
  }
  Polynomial * tmp = build_poly(0);
  return tmp;
}

/*------------------------------------------------------------------------*/
