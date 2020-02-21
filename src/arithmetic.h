//
// Created by robin on 2018/9/1.
// Copyright (c) 2018 Robin. All rights reserved.
//

#pragma once

#include <gsl/gsl>

namespace satz {

template <typename P>
const P zero = P::ZERO;

template <typename P>
const P one = P::ONE;

template <typename P>
const P X = P::X;

/**
 * @brief Compute $b^e$.
 * @param b base
 * @param e exponent
 * @param acc
 * @pre $e \ge 0, acc = 1$.
 */
template <typename P, typename N>
P pow (P b, N e, P acc = one<P>) {
  Expects(e >= 0);

  while (e) {
    if (e % 2) acc = acc * b;
    b = b * b;
    e = e / 2;
  }

  return acc;
}

/**
 * @brief Compute $(b^e) mod m$.
 * @param b base
 * @param e exponent
 * @param m modulus
 * @param acc
 * @pre $e >= 0, acc = 1$
 */
template <typename P, typename N>
P mod_pow (P b, N e, const P& m, P acc = one<P>) {
  Expects(e >= 0);

  while (e) {
    if (e % 2) acc = (acc * b) % m;
    b = (b * b) % m;
    e = e / 2;
  }

  return acc;
}

template <typename P>
P gcd (P a, P b) {
  while (b) {
    std::tie(a, b) = std::make_pair(b, a % b);
  }
  return a;
}

/**
 * @brief Compute $(x^{2^p} - x) mod m$.
 * @param p
 * @param m modulus
 */
template <typename P, typename N>
P reduce_exponent (N p, const P& m) {
  auto r = X<P>;
  while (p-- > 0) r = (r * r) % m;
  return (r - X<P>) % m;
}

/**
 * @brief Given a polynomial p over GF(2) return whether it is irreducible
 *    with Ben-Or's algorithm.
 * @param p polynomial over GF(2)
 * @return true if p is irreducible.
 * @pre p is non-constant.
 */
template <typename P>
bool ben_or_test (const P& p) {
  Expects(p.degree() > 0);

  auto             d = p.degree();
  for (decltype(d) i = 1; i <= d / 2; ++i) {
    auto b = reduce_exponent(i, p);
    auto g = gcd(p, b);
    if (g != one<P>) return false;
  }
  return true;
}

/**
 * @brief Given a polynomial p over GF(2), return whether it is irreducible.
 * @param p polynomial over GF(2)
 * @return true if p is irreducible.
 */
template <typename P>
bool is_irreducible (const P& p) {
  if (p.degree() > 0) return ben_or_test(p);
  else return false;
}

}

