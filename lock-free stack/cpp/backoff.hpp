#pragma once

#include <chrono>
#include <iostream>

namespace {

constexpr std::chrono::microseconds START{25};

} // namespace

class Backoff {
 public:
  std::chrono::microseconds Get() {
    return current_ *= 2;
  }
  void Reset() {
    current_ = START;
  }
 private:
  std::chrono::microseconds current_{START};
};
