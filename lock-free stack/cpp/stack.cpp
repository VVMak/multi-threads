#include "stack.hpp"
#include "backoff.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <thread>

thread_local Backoff backoff;

Stack::Stack(std::size_t num_of_threads) : num_of_threads_(num_of_threads) {
  for (std::size_t i = 0; i < num_of_threads_; ++i) {
    hazard_[i] = nullptr;
    elim_op_[i] = 0;
    elim_value_[i] = nullptr;
  }  
}

inline Stack::Node* Stack::LoadHead(std::size_t thread_num) {
  do {
    hazard_[thread_num].store(head_.load(std::memory_order_relaxed), std::memory_order_seq_cst);
  } while (hazard_[thread_num].load(std::memory_order_relaxed) != head_.load(std::memory_order_relaxed));
  return hazard_[thread_num].load(std::memory_order_relaxed);
}

void Stack::Push(int x, std::size_t thread_num) {
  std::mt19937 gen(thread_num);
  std::uniform_int_distribution<std::size_t> distr{0, (num_of_threads_ - 1) / 2};
  backoff.Reset();
  std::size_t elim_num = distr(gen);
  bool elim_busy = false;

  Node* new_head = new Node{x, LoadHead(thread_num)};
  while (!head_.compare_exchange_weak(new_head->next, new_head, std::memory_order_relaxed)) {
    if (elim_locked_[elim_num].TryLock()) {
      if (elim_op_[elim_num] == 0) {
        elim_op_[elim_num] = 1;
        elim_value_[elim_num] = new_head;
        elim_busy = true;
      } else if (elim_op_[elim_num] == -1 && elim_value_[elim_num] == nullptr) {
        elim_value_[elim_num] = new_head;
        elim_locked_[elim_num].Unlock();
        break;
      }
      elim_locked_[elim_num].Unlock();
    }

    std::this_thread::sleep_for(backoff.Get());
    // for (volatile std::size_t i = 0; i < 1000; ++i) {;}

    if (elim_busy) {
      elim_locked_[elim_num].Lock();
      elim_op_[elim_num] = 0;
      if (elim_value_[elim_num] == nullptr) {
        elim_locked_[elim_num].Unlock();
        break;
      }
      elim_value_[elim_num] = nullptr;
      elim_locked_[elim_num].Unlock();
      elim_busy = false;
    }

    new_head->next = LoadHead(thread_num);
  }
  hazard_[thread_num].store(nullptr, std::memory_order_release);
}

std::optional<int> Stack::Pop(std::size_t thread_num) {
  std::mt19937 gen(thread_num);
  std::uniform_int_distribution<std::size_t> distr{0, (num_of_threads_ - 1) / 2};
  backoff.Reset();
  std::size_t elim_num = distr(gen);
  bool elim_busy = false;

  Node* cur_head = LoadHead(thread_num);
  if (cur_head == nullptr) { return {}; }
  while (!head_.compare_exchange_weak(cur_head, cur_head->next, std::memory_order_relaxed)) {
    if (elim_locked_[elim_num].TryLock()) {
      if (elim_op_[elim_num] == 0) {
        elim_op_[elim_num] = -1;
        elim_busy = true;
      } else if (elim_op_[elim_num] == 1 && elim_value_[elim_num] != nullptr) {
        cur_head = elim_value_[elim_num];
        elim_value_[elim_num] = nullptr;
        elim_locked_[elim_num].Unlock();
        break;
      }
      elim_locked_[elim_num].Unlock();
    }

    std::this_thread::sleep_for(backoff.Get());
    // for (volatile std::size_t i = 0; i < 1000; ++i) {;}

    if (elim_busy) {
      elim_locked_[elim_num].Lock();
      elim_op_[elim_num] = 0;
      if (elim_value_[elim_num] != nullptr) {
        cur_head = elim_value_[elim_num];
        elim_value_[elim_num] = nullptr;
        elim_locked_[elim_num].Unlock();
        break;
      }
      elim_locked_[elim_num].Unlock();
      elim_busy = false;
    }

    cur_head = LoadHead(thread_num);
    if (cur_head == nullptr) { return {}; }
  }
  hazard_[thread_num].store(nullptr, std::memory_order_release);

  int value = cur_head->value;
  if (elim_busy) {
    delete cur_head;
    return value;
  }
  for (std::size_t i = 0; i < num_of_threads_; ++i) {
    if (i == thread_num) { continue; }
    if (hazard_[i].load(std::memory_order_relaxed) == cur_head) {
      std::size_t pos = free_list_size_.fetch_add(1, std::memory_order_acquire);
      while (pos >= FREE_LIST_CAPACITY) {
        while (free_list_size_.load(std::memory_order_relaxed) >= FREE_LIST_CAPACITY) {
          // std::this_thread::yield();
        }
        pos = free_list_size_.fetch_add(1, std::memory_order_acquire);
      }
      free_list_[pos] = cur_head;
      if (pos + 1 == FREE_LIST_CAPACITY) {
        FreeList();
      }
      return value;
    }
  }
  delete cur_head;
  return value;
}

void Stack::FreeList() {
  std::vector<Node*> new_free_list;
  new_free_list.reserve(FREE_LIST_CAPACITY);
  std::vector<Node*> hazard_copy;
  hazard_copy.reserve(num_of_threads_);
  for (std::size_t i = 0; i < num_of_threads_; ++i) {
    auto node = hazard_[i].load(std::memory_order_relaxed);
    if (node) {
      hazard_copy.push_back(hazard_[i].load(std::memory_order_relaxed));
    }
  }
  std::sort(hazard_copy.begin(), hazard_copy.end());
  for (auto& node : free_list_) {
    while (!node) {}
    if (std::binary_search(hazard_copy.begin(), hazard_copy.end(), node)) {
      new_free_list.push_back(node);
    } else {
      delete node;
    }
  }
  size_t new_size = new_free_list.size();
  free_list_ = std::move(new_free_list);
  free_list_.resize(FREE_LIST_CAPACITY, nullptr);

  // всегда будут какие-то элементы удаляться, т.к. размер free_list_ больше,
  // чем максимальное количество hazard pointers (последних не больше, чем число потоков)
  free_list_size_.store(new_size, std::memory_order_release);
} 

Stack::~Stack() {
  while (Pop(0)) {}
  for (int i = 0; i < free_list_size_.load(); ++i) {
    delete free_list_[i];
  }
}