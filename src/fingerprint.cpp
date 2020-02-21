//
// Created by robin on 2018/9/2.
// Copyright (c) 2018 Robin. All rights reserved.
//

#include "fingerprint.h"
#include "polynomial.h"

#include <iostream>

namespace satz::rabin {

FingerprintGenerator::FingerprintGenerator
    (FingerprintGenerator::value_type m) : m_(m) {

  if (!m_) return;

  lookup_a_.assign(256, 0);
  lookup_b_.assign(256, 0);
  lookup_c_.assign(256, 0);
  lookup_d_.assign(256, 0);

  using satz::gf2::Polynomial;

  // restore the irreducible polynomial
  auto poly_m = Polynomial::from_ulong(m_);
  auto leading = one<Polynomial> << (sizeof(m_) * 8);
  const auto p = leading + poly_m;

  // precompute the lookup tables
  for (unsigned int i = 0; i < 256; ++i) {
    auto a = Polynomial::from_ulong(i) << 88;
    lookup_a_[i] = bytes::from_bytes<value_type>((a % p).to_bytes());
  }

  for (unsigned int i = 0; i < 256; ++i) {
    auto b = Polynomial::from_ulong(i) << 80;
    lookup_b_[i] = bytes::from_bytes<value_type>((b % p).to_bytes());
  }

  for (unsigned int i = 0; i < 256; ++i) {
    auto c = Polynomial::from_ulong(i) << 72;
    lookup_c_[i] = bytes::from_bytes<value_type>((c % p).to_bytes());
  }

  for (unsigned int i = 0; i < 256; ++i) {
    auto d = Polynomial::from_ulong(i) << 64;
    lookup_d_[i] = bytes::from_bytes<value_type>((d % p).to_bytes());
  }
}

bool operator == (
    const FingerprintGenerator& lhs,
    const FingerprintGenerator& rhs) {return lhs.m_ == rhs.m_;}

bool operator != (
    const FingerprintGenerator& lhs,
    const FingerprintGenerator& rhs) {return !(rhs == lhs);}

std::pair<FingerprintGenerator, Fingerprint> FingerprintGenerator::create () {
  using satz::gf2::Polynomial;
  const auto p = Polynomial::make_irreducible(sizeof(value_type) * 8);
  auto m = satz::bytes::from_bytes<value_type>(p.to_bytes());
  return {FingerprintGenerator(m), Fingerprint(~0)};
}

}
