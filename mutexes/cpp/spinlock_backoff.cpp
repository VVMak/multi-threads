#include "backoff.hpp"
#include "spinlock_backoff.hpp"

#include <thread>

namespace {

thread_local Backoff backoff;

} // namespace

SpinlockBackoff::~SpinlockBackoff() {}

void SpinlockBackoff::Lock() {
  while (locked_.exchange(true, std::memory_order_acquire)) {
    while (locked_.load(std::memory_order_relaxed)) {
      std::this_thread::sleep_for(backoff.Get());
    }
  }
  backoff.Reset();
}

void SpinlockBackoff::Unlock() {
  locked_.store(false, std::memory_order_release);
}
