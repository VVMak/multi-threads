#include "gen.hpp"


Operation OpGen::GenerateOperation() {
  return (op_(gen_) ? Operation::Push : Operation::Pop);
}

int OpGen::GenerateValue() {
  return value_(gen_);
}

std::pair<Operation, std::optional<int>> OpGen::NextWithOp(Operation op) {
  return {op, (op == Push ? std::make_optional(GenerateValue()) : std::nullopt)};
}