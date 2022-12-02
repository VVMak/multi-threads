#pragma once

#include "gen.hpp"

#include <random>
#include <mutex>

class DisbalancedGen : public OpGen {
 public:
  DisbalancedGen();
  std::pair<Operation, std::optional<int>> Next() override;
 private:
  std::poisson_distribution<int> num_gen_;
  Operation current_op_;
  std::size_t num_of_operations_;
  std::mutex mutex_;
};