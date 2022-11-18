#pragma once

#include <chrono>

namespace {

constexpr std::chrono::microseconds START{10};

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
