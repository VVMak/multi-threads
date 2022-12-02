#include "balanced.hpp"

std::pair<Operation, std::optional<int>> BalancedGen::Next() {
  std::lock_guard guard(mutex_);
  Operation op = GenerateOperation();
  return {op, (op == Push ? std::make_optional(GenerateValue()) : std::nullopt)};
}
