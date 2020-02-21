//
// Created by robin on 2018/9/1.
// Copyright (c) 2018 Robin. All rights reserved.
//


#include "bytes.h"
#include <random>

namespace satz::bytes {

std::vector<uint8_t> make_random_bytes (int n) {
  Expects(n >= 0);

  using random_byte_engine
  = std::independent_bits_engine<std::default_random_engine, 8, uint8_t>;

  static auto engine =
                  [] () {
                    random_byte_engine e;
//        e.seed();
                    e.seed(std::random_device{}());
                    return e;
                  }();

  std::vector<uint8_t> bytes(n);
  std::generate(bytes.begin(), bytes.end(), std::ref(engine));

  return bytes;
}

}

