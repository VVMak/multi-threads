#pragma once

#include "mutex.hpp"

#include <atomic>

class Ticketlock : public Mutex {
 protected:
  std::atomic<uint32_t> current_ticket_{0};
  std::atomic<uint32_t> new_ticket_{0};
  inline void WaitForUnlock();
 public:
  void Lock() override;
  void Unlock() override;
  ~Ticketlock() override;
};
