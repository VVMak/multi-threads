#include "backoff.hpp"
#include "ticketlock_backoff.hpp"

#include <thread>
#include <iostream>

namespace {

thread_local Backoff backoff;

} // namespace

TicketlockBackoff::~TicketlockBackoff() {}

void TicketlockBackoff::Lock() {
  uint32_t ticket = new_ticket_.fetch_add(1, std::memory_order_relaxed);
  while (ticket > current_ticket_.load(std::memory_order_acquire)) {
    std::this_thread::sleep_for(backoff.Get());
  }
  backoff.Reset();
}

void TicketlockBackoff::Unlock() {
  current_ticket_.fetch_add(current_ticket_.load(std::memory_order_relaxed) + 1, std::memory_order_release);
}
