#pragma once

#include "gen.hpp"

#include <mutex>

class BalancedGen : public OpGen {
 public:
  std::pair<Operation, std::optional<int>> Next() override;
 private:
  std::mutex mutex_;
};