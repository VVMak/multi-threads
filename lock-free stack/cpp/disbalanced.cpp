#include "disbalanced.hpp"

DisbalancedGen::DisbalancedGen() : num_gen_{30}, current_op_{Pop}, num_of_operations_(0) {}

std::pair<Operation, std::optional<int>> DisbalancedGen::Next() {
  std::lock_guard guard(mutex_);
  if (num_of_operations_ == 0) {
    current_op_= static_cast<Operation>(-current_op_);
    num_of_operations_ = num_gen_(gen_);
  } else {
    --num_of_operations_;
  }
  return NextWithOp(current_op_);
}