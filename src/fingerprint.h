//
// Created by robin on 2018/9/2.
// Copyright (c) 2018 Robin. All rights reserved.
//

#pragma once

#include <cstdint>
#include <ostream>
#include <vector>
#include <utility>
#include <boost/type_index.hpp>
#include "bytes.h"

namespace satz::rabin {

using Fingerprint = uint64_t;

/**
 *
 * References:
 *      Fingerprinting by random polynomials, Rabin
 *      Some applications of Rabin's fingerprinting method, Broder
 *
 */

class FingerprintGenerator {
public:
  struct naive_one_byte_tag { };
  struct one_byte_tag { };
  struct four_byte_tag { };

  using value_type = Fingerprint;
  static_assert(sizeof(value_type) == 8);

  FingerprintGenerator () = default;
  FingerprintGenerator (const FingerprintGenerator&) = default;
  FingerprintGenerator (FingerprintGenerator&&) = default;
  FingerprintGenerator& operator = (const FingerprintGenerator&) = default;
  FingerprintGenerator& operator = (FingerprintGenerator&&) = default;

  /**
   * @brief Creates a fingerprint generator and the initial fingerprint.
   * @return (fingerprint generator, initial fingerprint) pair
   */
  static std::pair<FingerprintGenerator, Fingerprint> create ();

  friend bool operator == (const FingerprintGenerator& lhs,
                           const FingerprintGenerator& rhs);

  friend bool operator != (const FingerprintGenerator& lhs,
                           const FingerprintGenerator& rhs);

  template <typename InputIt>
  Fingerprint operator () (Fingerprint fp, InputIt first, InputIt last) const {
    using T = decltype(*first);
    static_assert(std::is_scalar_v<std::decay_t<T>>);

    if constexpr (sizeof(T) == 4) {
      return (*this)(fp, first, last, four_byte_tag{});
    } else if constexpr (sizeof(T) == 1) {
      return (*this)(fp, first, last, one_byte_tag{});
    } else {
      return std::accumulate(first, last, fp, *this);
    }
  }

  template <typename T>
  Fingerprint operator () (Fingerprint fp, T value) const {
    static_assert(std::is_scalar_v<std::decay_t<T>>);
    if constexpr (sizeof(value) % 4 == 0) {
      const uint32_t* first = reinterpret_cast<uint32_t*>(&value);
      return (*this)(fp,
                     std::make_reverse_iterator(first + sizeof(value) / 4),
                     std::make_reverse_iterator(first),
                     four_byte_tag{});
    } else {
      const uint8_t* first = reinterpret_cast<uint8_t*>(&value);
      return (*this)(fp,
                     std::make_reverse_iterator(first + sizeof(value)),
                     std::make_reverse_iterator(first),
                     one_byte_tag{});
    }
  }

protected:
  template <typename InputIt>
  Fingerprint operator () (
      Fingerprint fp, InputIt first, InputIt last, naive_one_byte_tag) const {

    static_assert(sizeof(decltype(*first)) == 1);

    auto binop = [m = m_] (Fingerprint fp, uint8_t b) -> Fingerprint {
      int i = 8;
      while (i-- > 0) {
        constexpr int shifts = sizeof(value_type) * 8 - 1;
        auto          msb    = bool(fp >> shifts);
        fp = (fp << 1) | ((b >> i) & 0x1);
        if (msb) fp = fp ^ m;
      }
      return fp;
    };

    return std::accumulate(first, last, fp, binop);
  }

  template <typename InputIt>
  Fingerprint operator () (
      Fingerprint fp, InputIt first, InputIt last, one_byte_tag) const {
    static_assert(sizeof(decltype(*first)) == 1);

    // The derivation of the formula below is similar to the one
    // used in Broder's paper.
    auto binop = [this] (Fingerprint fp, uint8_t b) -> Fingerprint {
      uint8_t* u8fp = reinterpret_cast<uint8_t*>(&fp);
      return ((fp << 8) | b) ^ lookup_d_[u8fp[7]];
    };
    return std::accumulate(first, last, fp, binop);
  }

  template <typename InputIt>
  Fingerprint operator () (
      Fingerprint fp, InputIt first, InputIt last, four_byte_tag) const {

    static_assert(sizeof(decltype(*first)) == 4);

    // We use the optimization technique in Broder's paper:
    //    Some applications of Rabin's fingerprinting method.
    // For details read Section 4 of that paper. It brings
    // us roughly 7~10x speedup for uint64_t, compared to
    // naive implementation.
    auto binop = [this] (Fingerprint fp, uint32_t x) -> Fingerprint {
      auto u32fp = reinterpret_cast<uint32_t*>(&fp);
      auto u8fp  = reinterpret_cast<uint8_t*>(&u32fp[1]);

      auto u32a = reinterpret_cast<const uint32_t*>(&lookup_a_[u8fp[3]]);
      auto u32b = reinterpret_cast<const uint32_t*>(&lookup_b_[u8fp[2]]);
      auto u32c = reinterpret_cast<const uint32_t*>(&lookup_c_[u8fp[1]]);
      auto u32d = reinterpret_cast<const uint32_t*>(&lookup_d_[u8fp[0]]);

      u32fp[1] = u32fp[0] ^ u32a[1] ^ u32b[1] ^ u32c[1] ^ u32d[1];
      u32fp[0] = x ^ u32a[0] ^ u32b[0] ^ u32c[0] ^ u32d[0];
      return fp;
    };
    return std::accumulate(first, last, fp, binop);
  }

protected:
  // `m` is the bit representation of an irreducible polynomial
  //    of degree `sizeof(value_type)*8` but with the leading bit
  //    removed (thus fitting into `value_type`)
  explicit FingerprintGenerator (value_type m);

private:
  value_type              m_ = 0;
  std::vector<value_type> lookup_a_;
  std::vector<value_type> lookup_b_;
  std::vector<value_type> lookup_c_;
  std::vector<value_type> lookup_d_;
};

}

