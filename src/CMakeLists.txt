cmake_minimum_required(VERSION 3.15)
project(rabin_fingerprint)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)
link_directories(/usr/local/lib)

add_executable(
        fingerprint.t
        fingerprint.t.cpp
        bytes.cpp
        fingerprint.cpp
        polynomial.cpp
)
target_link_libraries(fingerprint.t gtest)