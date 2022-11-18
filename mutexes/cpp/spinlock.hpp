#pragma once

#include "backoff.hpp"
#include "mutex.hpp"

#include <atomic>

class Spinlock : public Mutex {
 protected:
  std::atomic<bool> locked_{false};
  inline void WaitForUnlock();
 public:
  void Lock() override;
  void Unlock() override;
  ~Spinlock() override;
};
