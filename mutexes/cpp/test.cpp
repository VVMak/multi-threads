#include "mutex.hpp"
#include "spinlock.hpp"
#include "ticketlock.hpp"
#include "spinlock_backoff.hpp"
#include "ticketlock_backoff.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <fstream>
#include <random>
#include <thread>
#include <vector>

#include <unistd.h>

class RandomTimeGenerator {
 private:
  std::mt19937 gen_;
  std::uniform_int_distribution<size_t> distr_;
 public:
  RandomTimeGenerator(size_t min = 5, size_t max = 500)
    : gen_(), distr_(min, max) {}
  std::chrono::microseconds Get() {
    return std::chrono::microseconds(distr_(gen_));
  }
};

// in mcsec
auto Test(size_t num_of_iterations, size_t num_of_threads, Mutex&& mutex) {
  std::vector<std::thread> threads;
  threads.reserve(num_of_threads);
  int64_t max_lock = 0;
  int64_t sum_lock = 0;
  for (size_t thread_num = 0; thread_num < num_of_threads; ++thread_num) {
    threads.emplace_back([num_of_iterations, &mutex, &max_lock, &sum_lock]() {
      std::chrono::steady_clock clock;
      RandomTimeGenerator g;
      for (size_t i = 0; i < num_of_iterations; ++i) {
        const auto start = clock.now();
        mutex.Lock();
        const auto lock_time = (clock.now() - start).count();
        max_lock = std::max(max_lock, lock_time);
        sum_lock += lock_time;
        usleep(g.Get().count());
        mutex.Unlock();
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  const auto average_lock = sum_lock / num_of_iterations / num_of_threads;
  return std::make_pair(average_lock / 1000, max_lock / 1000);
}

constexpr size_t ITERATIONS = 500;

int main() {
  std::ofstream fstrm("data.txt");
  for (size_t threads_num = 1; threads_num <= 16; ++threads_num) {
    auto [average_spinlock, max_spinlock] = Test(ITERATIONS, threads_num, Spinlock());
    fstrm << average_spinlock << ' ' << max_spinlock << ' ';
    auto [average_ticketlock, max_ticketlock] = Test(ITERATIONS, threads_num, Ticketlock());
    fstrm << average_ticketlock << ' ' << max_ticketlock << ' ';
    auto [average_spinlock_b, max_spinlock_b] = Test(ITERATIONS, threads_num, SpinlockBackoff());
    fstrm << average_spinlock_b << ' ' << max_spinlock_b << ' ';
    auto [average_ticketlock_b, max_ticketlock_b] = Test(ITERATIONS, threads_num, TicketlockBackoff());
    fstrm << average_ticketlock_b << ' ' << max_ticketlock_b << std::endl;
  }
  return 0;
}