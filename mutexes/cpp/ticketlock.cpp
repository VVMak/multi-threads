#include "ticketlock.hpp"

#include <thread>

Ticketlock::~Ticketlock() {}

void Ticketlock::Lock() {
  uint32_t ticket = new_ticket_.fetch_add(1, std::memory_order_relaxed);
  while (ticket > current_ticket_.load(std::memory_order_acquire)) {
    std::this_thread::yield();
    __asm volatile("pause" ::: "memory");
  }
}

void Ticketlock::Unlock() {
  current_ticket_.store(current_ticket_.load(std::memory_order_relaxed) + 1, std::memory_order_release);
}
