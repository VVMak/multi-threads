#pragma once

#include <mutex>
#include <optional>
#include <random>
#include <utility>
#include <vector>

#include "operation.hpp"


class OpGen {
 public:
  virtual std::pair<Operation, std::optional<int>> Next() = 0;
  virtual ~OpGen() {}
 protected:
  Operation GenerateOperation();
  int GenerateValue();
  std::pair<Operation, std::optional<int>> NextWithOp(Operation op);

  std::mt19937 gen_;
 private:
  std::bernoulli_distribution op_;
  std::uniform_int_distribution<int> value_{-10, 10};
};
