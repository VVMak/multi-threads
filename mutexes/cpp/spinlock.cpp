#include "spinlock.hpp"

#include <thread>

Spinlock::~Spinlock() {}

void Spinlock::Lock() {
  while (locked_.exchange(true, std::memory_order_acquire)) {
    while (locked_.load(std::memory_order_relaxed)) {
      std::this_thread::yield();
      __asm volatile("pause" ::: "memory");
    }
  }
}

void Spinlock::Unlock() {
  locked_.store(false, std::memory_order_release);
}
