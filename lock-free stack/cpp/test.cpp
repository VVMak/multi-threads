#include "balanced.hpp"
#include "disbalanced.hpp"
#include "stack.hpp"

#include <fstream>
#include <memory>
#include <thread>

std::pair<int, int> TestStack(std::size_t num_of_threads, OpGen&& gen, std::size_t num_of_iter = 20000) {
  auto stack = std::make_shared<Stack>(num_of_threads);
  std::vector<std::thread> threads;
  threads.reserve(num_of_threads);
  int64_t max_op = 0;
  int64_t sum_op = 0;
  std::mutex mutex;
  for (size_t thread_num = 0; thread_num < num_of_threads; ++thread_num) {
    threads.emplace_back([&, thread_num, stack]() {
      std::chrono::steady_clock clock;
      for (size_t i = 0; i < num_of_iter; ++i) {
        const auto [op_type, value] = gen.Next();
        const auto start = clock.now();
        if (op_type == Push) {
          stack->Push(*value, thread_num);
        } else {
          stack->Pop(thread_num);
        }
        const auto op_time = (clock.now() - start).count();
        mutex.lock();
        max_op = std::max(max_op, op_time);
        sum_op += op_time;
        mutex.unlock();
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  const auto average_lock = sum_op / num_of_iter / num_of_threads;
  return std::make_pair(average_lock, max_op / 1000);
}

constexpr std::size_t REPEATS = 10;

int main() {
  std::ofstream fstrm("data.txt");
  for (size_t threads_num = 1; threads_num <= MAX_THREADS; ++threads_num) {
    int average_balanced = 0; int max_balanced = 0;
    int average_disbalanced = 0; int max_disbalanced = 0;
    for (size_t repeats_num = 0; repeats_num < REPEATS; ++repeats_num) {
      auto [average_balanced_temp, max_balanced_temp] = TestStack(threads_num, BalancedGen());
      average_balanced += average_balanced_temp; max_balanced += max_balanced_temp;
      auto [average_disbalanced_temp, max_disbalanced_temp] = TestStack(threads_num, DisbalancedGen());
      average_disbalanced += average_disbalanced_temp; max_disbalanced += max_disbalanced_temp;
    }
    average_balanced /= REPEATS; max_balanced /= REPEATS;
    average_disbalanced /= REPEATS; max_disbalanced /= REPEATS;
    fstrm << average_balanced  << ' ' << max_balanced << ' ';
    fstrm << average_disbalanced << ' ' << max_disbalanced << std::endl;
  }
  return 0;
}