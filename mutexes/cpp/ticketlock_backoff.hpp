#pragma once

#include "mutex.hpp"

#include <atomic>

class TicketlockBackoff : public Mutex {
 protected:
  std::atomic<uint32_t> current_ticket_{0};
  std::atomic<uint32_t> new_ticket_{0};
 public:
  void Lock() override;
  void Unlock() override;
  ~TicketlockBackoff() override;
};
