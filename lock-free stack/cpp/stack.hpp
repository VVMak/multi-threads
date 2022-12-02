#pragma once

#include <atomic>
#include <optional>
#include <vector>

#include "spinlock.hpp"

constexpr std::size_t MAX_THREADS = 16;
constexpr std::size_t FREE_LIST_CAPACITY = 32;


class Stack {
 public:
  Stack(std::size_t num_of_threads);
  void Push(int x, std::size_t thread_num);
  std::optional<int> Pop(std::size_t thread_num);
  ~Stack();
 private:
  struct Node {
    int value;
    Node* next;
  };
  int elim_op_[MAX_THREADS];
  Node* elim_value_[MAX_THREADS];
  Spinlock elim_locked_[MAX_THREADS];

  std::atomic<Node*> hazard_[MAX_THREADS];

  std::vector<Node*> free_list_{FREE_LIST_CAPACITY, nullptr};
  std::atomic<int> free_list_size_{0};
  void FreeList();
  

  std::atomic<Node*> head_ = nullptr;
  inline Node* LoadHead(std::size_t thread_num);

  std::size_t num_of_threads_;

};
