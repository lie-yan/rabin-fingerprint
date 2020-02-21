//
// Created by robin on 2018/9/3.
// Copyright (c) 2018 Robin. All rights reserved.
//

#pragma once

#include <chrono>

namespace satz {

struct measure {

  template<typename F, typename ...Args>
  static std::chrono::milliseconds::rep ms (F f, Args&& ... args) {
    using std::chrono::milliseconds;
    return elapsed_time<milliseconds>(f, std::forward<Args>(args)...);
  }

  template<typename F, typename ...Args>
  static std::chrono::microseconds::rep us (F f, Args&& ... args) {
    using std::chrono::microseconds;
    return elapsed_time<microseconds>(f, std::forward<Args>(args)...);
  }

  template<typename F, typename ...Args>
  static std::chrono::nanoseconds::rep ns (F f, Args&& ... args) {
    using std::chrono::nanoseconds;
    return elapsed_time<nanoseconds>(f, std::forward<Args>(args)...);
  }

private:
  template<typename U, typename F, typename ...Args>
  static typename U::rep elapsed_time (F f, Args&& ... args) {
    using namespace std::chrono;
    auto start = system_clock::now();
    f(std::forward<Args>(args)...);
    auto stop = system_clock::now();
    return duration_cast<U>(stop - start).count();
  }
};

}

