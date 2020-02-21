//
//  fingerprint.t.cpp
//
//  Created by robin on 2018/9/2.
//  Copyright © 2018 Robin. All rights reserved.
//


#include <iostream>
#include <array>
#include "fingerprint.h"
#include "measure.h"
#include "gtest/gtest.h"

namespace {

TEST(Fingerprint, correctness) {

  using namespace satz::rabin;

  FingerprintGenerator fg;
  Fingerprint fp1;

  std::tie(fg, fp1) = FingerprintGenerator::create();

  Fingerprint fp2 = fp1;
  Fingerprint fp3 = fp1;

  fp1 = fg(fp1, 0xdead0000feedbeefULL);

  fp2 = fg(fp2, uint8_t(0xde));
  fp2 = fg(fp2, uint8_t(0xad));
  fp2 = fg(fp2, uint8_t(0x00));
  fp2 = fg(fp2, uint8_t(0x00));
  fp2 = fg(fp2, uint8_t(0xfe));
  fp2 = fg(fp2, uint8_t(0xed));
  fp2 = fg(fp2, uint8_t(0xbe));
  fp2 = fg(fp2, uint8_t(0xef));

  EXPECT_EQ(fp1, fp2);

  std::array<uint16_t, 4> u16example = {0xdead, 0x0000, 0xfeed, 0xbeef};
  fp3 = fg(fp3, u16example.begin(), u16example.end());
  EXPECT_EQ(fp1, fp3);
}

TEST(Fingerprint, four_byte_speed) {
  using namespace satz::rabin;
  using satz::measure;

  FingerprintGenerator fg;
  Fingerprint fp = 0;

  auto init_op = [&] () {
    std::tie(fg,fp) = FingerprintGenerator::create();
  };

  auto milliseconds = satz::measure::ms(std::ref(init_op));
  std::cout << "four_byte_speed (construction): " << milliseconds << "ms\n";

  int count = 3000'000;
  auto consume_op = [&] () {
    for (int i = 0; i < count; ++i) {
      fp = fg(fp, uint64_t(i));
    }
  };
  auto nanoseconds = measure::ns(std::ref(consume_op));
  double op_time = (1.0 * nanoseconds / count);
  std::cout << "four_byte_speed (generation): " << op_time << "ns per call\n";
  EXPECT_LE(op_time, 100);
}

}

int main (int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
