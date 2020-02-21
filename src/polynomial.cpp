//
// Created by robin on 2018/9/1.
// Copyright (c) 2018 Robin. All rights reserved.
//

#include "polynomial.h"
#include "bytes.h"

#include <boost/lexical_cast.hpp>
#include <experimental/iterator>

namespace satz::gf2::v2 {

Polynomial Polynomial::from_ulong (unsigned long l) {
  static_assert(sizeof(l) == 8);
  uint8_t* start = reinterpret_cast<uint8_t*>(&l);
  return from_bytes({start, start + sizeof(l)});
}

Polynomial Polynomial::from_bytes (gsl::span<uint8_t> bytes) {
  return Polynomial(satz::bytes::make_indices<int_type>(bytes));
}

Polynomial Polynomial::from_bytes (gsl::span<uint8_t> bytes,
    Polynomial::int_type degree) {

  Expects(bytes.size() * 8 > degree);

  std::vector<int_type> d = satz::bytes::make_indices<int_type>(bytes);
  auto it = std::lower_bound(d.begin(), d.end(), degree);
  if (it == d.end() || *it != degree) {
    d.erase(it, d.end());
    d.push_back(static_cast<int>(degree));
  } else { // `*it == degree`
    std::advance(it, 1);
    d.erase(it, d.end());
  }

  return Polynomial(std::move(d));
}

Polynomial Polynomial::make_random (Polynomial::int_type degree) {
  auto bytes = satz::bytes::make_random_bytes(static_cast<int>(degree / 8 + 1));
  return Polynomial::from_bytes(bytes, degree);
}

Polynomial Polynomial::make_irreducible (Polynomial::int_type degree) {
  Expects(degree > 0);

  // According to
  //    Some applications of Rabin’s fingerprinting method [Broder],
  // the total number of irreducible polynomials of degree $k$ with
  // coefficients in $Z_2$ is greater than $(2^k - 2^{k/2})/k$. Thus
  // the probability of picking an irreducible polynomial with a
  // uniformly random draw is roughly greater than $1/k$. If we try
  // $m$ independent draws, the probability of failing to obtain an
  // irreducible polynomial is roughly less than $(1 - 1/k)^m$, which
  // is roughly $e^{-m/k}$.
  int magic_number = 15;
  auto trials = degree * magic_number;
  while (trials-- > 0) {
    auto p = Polynomial::make_random(degree);
    if (is_irreducible(p)) return p;
  }

  throw std::runtime_error("fail to obtain an irreducible polynomial");
}

std::vector<uint8_t> Polynomial::to_bytes () const {
  const int d = degree();
  const size_t count = (d >= 0) ? (d / 8 + 1) : 0;

  std::vector<uint8_t> ret(count, 0);
  std::for_each(deg_.begin(),
                deg_.end(),
                [&] (int_type n) {
                  const auto i = n / 8;
                  const auto j = n % 8;
                  ret[i] = ret[i] | uint8_t(0x1 << j);
                });
  return ret;
}

Polynomial::operator bool () const {return !empty();}

bool operator == (const Polynomial& lhs, const Polynomial& rhs) {
  return lhs.deg_ == rhs.deg_;
}

bool operator != (const Polynomial& lhs, const Polynomial& rhs) {
  return !(rhs == lhs);
}

Polynomial& Polynomial::operator ^= (const Polynomial& rhs) {
  Polynomial res = *this ^rhs;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial& Polynomial::operator |= (const Polynomial& rhs) {
  Polynomial res = *this | rhs;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial& Polynomial::operator &= (const Polynomial& rhs) {
  Polynomial res = *this & rhs;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial& Polynomial::operator += (const Polynomial& rhs) {
  Polynomial res = *this + rhs;
  deg_.swap(res.deg_);
  return *this;

}

Polynomial& Polynomial::operator -= (const Polynomial& rhs) {
  Polynomial res = *this - rhs;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial& Polynomial::operator *= (const Polynomial& rhs) {
  Polynomial res = *this * rhs;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial& Polynomial::operator %= (const Polynomial& rhs) {
  Polynomial res = *this % rhs;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial& Polynomial::operator <<= (Polynomial::int_type n) {
  Expects(n >= 0);
  Polynomial res = *this << n;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial& Polynomial::operator >>= (Polynomial::int_type n) {
  Expects(n >= 0);
  Polynomial res = *this >> n;
  deg_.swap(res.deg_);
  return *this;
}

Polynomial operator ^ (const Polynomial& lhs, const Polynomial& rhs) {
  Polynomial ret;
  std::set_symmetric_difference(lhs.deg_.begin(),
                                lhs.deg_.end(),
                                rhs.deg_.begin(),
                                rhs.deg_.end(),
                                std::back_inserter(ret.deg_));
  return ret;
}

Polynomial operator | (const Polynomial& lhs, const Polynomial& rhs) {
  Polynomial ret;
  std::set_union(lhs.deg_.begin(),
                 lhs.deg_.end(),
                 rhs.deg_.begin(),
                 rhs.deg_.end(),
                 std::back_inserter(ret.deg_));
  return ret;
}

Polynomial operator & (const Polynomial& lhs, const Polynomial& rhs) {
  Polynomial ret;
  std::set_intersection(lhs.deg_.begin(),
                        lhs.deg_.end(),
                        rhs.deg_.begin(),
                        rhs.deg_.end(),
                        std::back_inserter(ret.deg_));
  return ret;
}

Polynomial operator + (const Polynomial& lhs, const Polynomial& rhs) {
  return lhs ^ rhs;
}

Polynomial operator - (const Polynomial& lhs, const Polynomial& rhs) {
  return lhs ^ rhs;
}

template<typename T, typename N>
static void flip (std::vector<T>& bytes, N n) {
  bytes[n / 8] ^= (1 << (n % 8));
}

Polynomial operator * (const Polynomial& lhs, const Polynomial& rhs) {
  if (lhs.degree() == 0 || rhs.degree() == 0) return Polynomial{};

  auto degree_sum = lhs.degree() + rhs.degree();
  std::vector<uint8_t> bytes(degree_sum / 8 + 1, 0);
  for (const auto& x : lhs.deg_)
    for (const auto& y : rhs.deg_)
      flip(bytes, x + y);
  return Polynomial::from_bytes(bytes);
}

Polynomial Polynomial::operator << (Polynomial::int_type n) const {
  Expects(n >= 0);

  Polynomial ret;
  std::transform(deg_.begin(),
                 deg_.end(),
                 std::back_inserter(ret.deg_),
                 [&n] (const auto& m) {return m + n;});
  return ret;
}

Polynomial Polynomial::operator >> (Polynomial::int_type n) const {
  Expects(n >= 0);

  Polynomial ret;
  auto start = std::find_if(deg_.begin(),
                            deg_.end(),
                            [&n] (const auto& m) {return m > n;});
  std::transform(start,
                 deg_.end(),
                 std::back_inserter(ret.deg_),
                 [&n] (const auto& m) {return m - n;});
  return ret;
}

Polynomial operator % (const Polynomial& lhs, const Polynomial& rhs) {

  Polynomial ret{lhs};

  using N = Polynomial::int_type;
  const N dl = lhs.degree();
  const N dr = rhs.degree();

  if (dl >= dr) {
    N i = dl - dr;
    do {
      if (ret.contains(i + dr)) ret -= (rhs << i);
    } while (N(0) != i--);
  }

  return ret;
}

std::ostream& operator << (std::ostream& os, const Polynomial& obj) {
  // @formatter:off
  auto to_string = [] (const auto& n) {
    return
      n > 1 ? ("x^" + boost::lexical_cast<std::string>(n)) :
      n == 1 ? "x" :
      n == 0 ? "1" :
      throw std::runtime_error("n should be nonnegative");
  };
  // @formatter:on

  const auto& d = obj.deg_;
  if (d.empty()) {
    os << "0";
  } else {
    std::transform(d.rbegin(),
                   d.rend(),
                   std::experimental::make_ostream_joiner(os, "+"),
                   to_string);
  }
  return os;
}

bool Polynomial::empty () const {return deg_.empty();}

Polynomial::int_type Polynomial::degree () const {
  return empty() ? -1 : *deg_.rbegin();
}

Polynomial::int_type Polynomial::nnz () const {
  return static_cast<int_type>(deg_.size());
}

bool Polynomial::contains (Polynomial::int_type n) const {
  return std::binary_search(deg_.begin(), deg_.end(), n);
}

Polynomial::Polynomial (Polynomial::container_type&& v) : deg_(std::move(v)) {
  Expects(std::all_of(deg_.begin(),
                      deg_.end(),
                      [] (const auto& n) {return n >= 0;}));
  Expects(std::is_sorted(deg_.begin(), deg_.end()));
}

bool operator < (const Polynomial& lhs, const Polynomial& rhs) {
  return std::lexicographical_compare(lhs.deg_.rbegin(),
                                      lhs.deg_.rend(),
                                      rhs.deg_.rbegin(),
                                      rhs.deg_.rend());
}

bool operator > (const Polynomial& lhs, const Polynomial& rhs) {
  return rhs < lhs;
}

bool operator <= (const Polynomial& lhs, const Polynomial& rhs) {
  return !(rhs < lhs);
}

bool operator >= (const Polynomial& lhs, const Polynomial& rhs) {
  return !(lhs < rhs);
}

}