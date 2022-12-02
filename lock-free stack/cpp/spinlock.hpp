#pragma once

#include <atomic>
#include <thread>

class Spinlock {
 public:
  bool TryLock() {
    return !locked_.exchange(true, std::memory_order_acquire);
  }

  void Lock() {
    while (!TryLock()) {
      while (locked_.load(std::memory_order_relaxed)) {
        std::this_thread::yield();
      }
    }
  }

  void Unlock() {
    locked_.store(false, std::memory_order_release);
  }
 private:
  std::atomic<int> locked_{false};
};
