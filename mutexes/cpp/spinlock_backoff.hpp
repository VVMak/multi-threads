#pragma once

#include "backoff.hpp"
#include "mutex.hpp"

#include <atomic>

class SpinlockBackoff : public Mutex {
 protected:
  std::atomic<bool> locked_{false};
 public:
  void Lock() override;
  void Unlock() override;
  ~SpinlockBackoff() override;
};
