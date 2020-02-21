//
// Created by robin on 2018/9/1.
// Copyright (c) 2018 Robin. All rights reserved.
//

#pragma once

#include <type_traits>
#include <vector>
#include <algorithm>
#include <gsl/gsl>

namespace satz::bytes {

/**
 * @brief Generate n random bytes.
 */
std::vector<uint8_t> make_random_bytes (int n);



/**
 * @brief Given a byte sequence, return the indices of non-zero bits, regarding
 *    the bytes as a little-endian bit array with index starting at 0.
 * @param bytes
 * @post The resultant vector should be in increasing order.
 */
template <typename T>
std::vector<T> make_indices (gsl::span<const uint8_t> bytes) {
  std::vector<T> d;
  d.reserve(bytes.size() * 8);

  for (int i = 0; i < bytes.size(); ++i) {
    for (int j = 0; j < 8; ++j) {
      if ((bytes[i] >> j) & 0x1) d.push_back(8 * i + j);
    }
  }

  return d;
}

/**
 * @brief Given a byte sequence, join the bytes into a single object.
 *    If the number of bytes is greater than the size of T, discard extra
 *    bytes. If the number of bytes is less than the size of T, pad with
 *    leading zeros.
 * @param bytes
 * @example
 *    [0xef, 0xbe, 0xed, 0xfe] ==> 0xfeedbeef
 *    [0xed, 0xfe] ==> 0x0000feed
 *    [0xef, 0xbe, 0xed, 0xfe, 0xee] => 0xfeedbeef
 * @post The relative byte order of the result should be the same as the argument.
 */
template <typename T>
T from_bytes (gsl::span<const uint8_t> bytes) {

  static_assert(std::is_trivially_copyable_v<T>);
  using U = std::make_unsigned_t<T>;
  static_assert(sizeof(U) == sizeof(T));

  U             ret = 0;
  const ssize_t m   = bytes.size();
  const ssize_t n   = sizeof(U);

  std::for_each(bytes.rbegin() + std::max(m - n, ssize_t(0)),
                bytes.rend(),
                [&ret] (uint8_t b) {
                  ret = (ret << 8) | b;
                });

  return static_cast<T>(ret);
}

}
