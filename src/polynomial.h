//
// Created by robin on 2018/9/1.
// Copyright (c) 2018 Robin. All rights reserved.
//

#pragma once

#include <ostream>
#include <algorithm>
#include <vector>
#include <gsl/gsl>
#include "arithmetic.h"

namespace satz::gf2::v2 {

/// @brief Polynomial over GF(2).
class Polynomial {
public:
  using int_type = int;
  using container_type = std::vector<int_type>;

  Polynomial () = default;

  template <typename T>
  Polynomial (std::initializer_list<T> degrees)
      : Polynomial(degrees.begin(), degrees.end()) { }

  template <typename InputIt>
  Polynomial (InputIt first, InputIt last) : deg_(first, last) {
    Expects(std::all_of(deg_.begin(),
                        deg_.end(),
                        [] (const auto& x) { return x >= 0; }));

    if (!std::is_sorted(deg_.begin(), deg_.end())) {
      std::sort(deg_.begin(), deg_.end());
    }
    deg_.erase(std::unique(deg_.begin(), deg_.end()), deg_.end());
  }

  static Polynomial from_ulong (unsigned long);
  static Polynomial from_bytes (gsl::span<uint8_t>);
  static Polynomial from_bytes (gsl::span<uint8_t>, int_type degree);
  static Polynomial make_random (int_type degree);
  static Polynomial make_irreducible (int_type degree);

  /**
   * @brief Return a bit representation of the coefficients.
   *
   *    When the degree of this polynomial is -1, return an empty vector.
   *    In other cases, return a bit representation with just enough bytes,
   *    i.e., the most significant byte should be non-zero.
   */
  [[nodiscard]] std::vector<uint8_t> to_bytes () const;

  explicit operator bool () const;

  friend bool operator == (const Polynomial& lhs, const Polynomial& rhs);
  friend bool operator != (const Polynomial& lhs, const Polynomial& rhs);
  friend bool operator < (const Polynomial& lhs, const Polynomial& rhs);
  friend bool operator > (const Polynomial& lhs, const Polynomial& rhs);
  friend bool operator <= (const Polynomial& lhs, const Polynomial& rhs);
  friend bool operator >= (const Polynomial& lhs, const Polynomial& rhs);
  Polynomial& operator ^= (const Polynomial& rhs);
  Polynomial& operator |= (const Polynomial& rhs);
  Polynomial& operator &= (const Polynomial& rhs);
  Polynomial& operator += (const Polynomial& rhs);
  Polynomial& operator -= (const Polynomial& rhs);
  Polynomial& operator *= (const Polynomial& rhs);
  Polynomial& operator %= (const Polynomial& rhs);
  Polynomial& operator <<= (int_type n);
  Polynomial& operator >>= (int_type n);

  friend Polynomial operator ^ (const Polynomial& lhs, const Polynomial& rhs);
  friend Polynomial operator | (const Polynomial& lhs, const Polynomial& rhs);
  friend Polynomial operator & (const Polynomial& lhs, const Polynomial& rhs);
  friend Polynomial operator + (const Polynomial& lhs, const Polynomial& rhs);
  friend Polynomial operator - (const Polynomial& lhs, const Polynomial& rhs);
  friend Polynomial operator * (const Polynomial& lhs, const Polynomial& rhs);
  friend Polynomial operator % (const Polynomial& lhs, const Polynomial& rhs);
  Polynomial operator << (int_type n) const;
  Polynomial operator >> (int_type n) const;

  friend std::ostream& operator << (std::ostream& os, const Polynomial& obj);

  /**
   * @brief Indicates whether it is a zero polynomial.
   */
  [[nodiscard]] bool empty () const;

  /**
   * @brief Returns the degree of the polynomial.
   */
  [[nodiscard]] int_type degree () const;

  /**
   * @brief Returns the number of non-zero coefficients.
   */
  [[nodiscard]] int_type nnz () const;

  /**
   * @brief Given an interger n, return whether this polynomial contains an
   *    x^n term.
   * @param n
   */
  [[nodiscard]] bool contains (int_type n) const;

protected:
  explicit Polynomial (container_type&& v);

private:
  container_type deg_; // `deg_` for degrees
};

}

namespace satz::gf2 {
using namespace v2;
}

namespace satz {

template <>
const gf2::v2::Polynomial zero<gf2::v2::Polynomial> = gf2::v2::Polynomial{};

template <>
const gf2::v2::Polynomial one<gf2::v2::Polynomial> = gf2::v2::Polynomial{0};

template <>
const gf2::v2::Polynomial X<gf2::v2::Polynomial> = gf2::v2::Polynomial{1};

}
