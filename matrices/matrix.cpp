#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>


namespace {
  constexpr size_t BLOCK_SIZE = 16;
}

class MatrixMultiplication{
  public:
    MatrixMultiplication(const std::string& filename, char mode, size_t num_of_threads);
    int Run();
    ~MatrixMultiplication();
  private:
    const char mode_;
    int* left_ = nullptr;
    int* right_ = nullptr;
    int* result_ = nullptr;
    size_t size_;
    size_t capacity_;
    size_t num_of_threads_;

    inline int& at(int* arr, size_t row, size_t col) { return arr[row * capacity_ + col]; }
    inline int& left(size_t row, size_t col) { return at(left_, row, col); }
    inline int& right(size_t row, size_t col) { return at(right_, row, col); }
    inline int& result(size_t row, size_t col) { return at(result_, row, col); }
    
    int* allocate_2d_array();

    void ReadArrayFrom(std::ifstream& f_in, int*& arr);
    void ReadFile(const std::string& filename);
    void NaiveSingleThread();
    void BlockSingleThread();
    void NaiveMultiThread();
    void BlockMultiThread();
};

MatrixMultiplication::MatrixMultiplication(const std::string& filename, char mode, size_t num_of_threads)
    : mode_(mode), num_of_threads_(num_of_threads) {
  if (mode_ < '1' || mode_ > '4') {
    throw std::invalid_argument("mode should be from 1 to 4" + mode_);
  }
  ReadFile(filename);
  result_ = allocate_2d_array();
}

MatrixMultiplication::~MatrixMultiplication() {
      if (left_) {
        std::free(left_);
      }
      if (right_) {
        std::free(right_);
      }
      if (result_) {
        std::free(result_);
      }
    }

int* MatrixMultiplication::allocate_2d_array() {
  void* alloc = std::malloc(sizeof(int) * capacity_ * capacity_);
  int* arr = reinterpret_cast<int*>(alloc);
  for (int i = 0; i < capacity_ * capacity_; ++i) {
    arr[i] = 0;
  }
  return arr;
}

void MatrixMultiplication::ReadArrayFrom(std::ifstream& f_in, int*& arr) {
  // malloc просто для выравнивания
  arr = allocate_2d_array();
  for (size_t i = 0; i < size_; ++i) {
    for (size_t j = 0; j < size_; ++j)
      f_in >> at(arr, i, j);
  }
}

void MatrixMultiplication::ReadFile(const std::string& filename) {
  std::ifstream f_in(filename);
  if (!f_in.is_open()) {
    throw std::invalid_argument("failed to open " + filename);
  }
  f_in >> size_;
  capacity_ = (size_ / 64 + static_cast<bool>(size_ & 63)) * 64;
  ReadArrayFrom(f_in, left_);
  ReadArrayFrom(f_in, right_);
}

int MatrixMultiplication::Run() {
  std::chrono::steady_clock clock;
  const auto t1 = clock.now();
  switch (mode_)
  {
  case '1':
    NaiveSingleThread();
    break;
  case '2':
    BlockSingleThread();
    break;
  case '3':
    NaiveMultiThread();
    break;
  case '4':
    BlockMultiThread();
    break;
  default:
    throw std::invalid_argument("mode should be from 1 to 4");
    break;
  }
  const auto t2 = clock.now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

void MatrixMultiplication::NaiveSingleThread() {
  for (size_t i = 0; i < size_; ++i) {
    for (size_t j = 0; j < size_; ++j) {
      for (size_t k = 0; k < size_; ++k) {
        result(i, j) += left(i, k) * right(k, j);
      }
    }
  }
}

void MatrixMultiplication::BlockSingleThread() {
  for (size_t b_row = 0; b_row < size_; b_row += BLOCK_SIZE) {
    for (size_t b_k = 0; b_k < size_; b_k += BLOCK_SIZE) {
      for (size_t b_col = 0; b_col < size_; b_col += BLOCK_SIZE) {
        for (size_t add_row = 0; add_row < BLOCK_SIZE; ++add_row) {
          for (size_t add_k = 0; add_k < BLOCK_SIZE; ++add_k) {
            for (size_t add_col = 0; add_col < BLOCK_SIZE; ++add_col) {
              result(b_row + add_row, b_col + add_col) += left(b_row + add_row, b_k + add_k) * right(b_k + add_k, b_col + add_col);
            }
          }
        }
      }
    }
  }
}

void MatrixMultiplication::NaiveMultiThread() {
  std::vector<std::thread> threads;
  threads.reserve(num_of_threads_);
  for (size_t thread_num = 0; thread_num < num_of_threads_; ++thread_num) {
    threads.emplace_back([this, thread_num] () {
      for (size_t i = thread_num; i < size_; i += num_of_threads_) {
        for (size_t j = 0; j < size_; ++j) {
          for (size_t k = 0; k < size_; ++k) {
            result(i, j) += left(i, k) * right(k, j);
          }
        }
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
}

void MatrixMultiplication::BlockMultiThread() {
  std::vector<std::thread> threads;
  threads.reserve(num_of_threads_);
  size_t blocks_in_row = capacity_ / BLOCK_SIZE;
  size_t num_of_blocks = blocks_in_row * blocks_in_row;
  for (size_t thread_num = 0; thread_num < num_of_threads_; ++thread_num) {
    threads.emplace_back([this, thread_num, num_of_blocks, blocks_in_row]() {
      for (size_t block_num = thread_num; block_num < num_of_blocks; block_num += num_of_threads_) {
        size_t block_row = block_num / blocks_in_row;
        size_t block_col = block_num % blocks_in_row;
        size_t row_begin = block_row * BLOCK_SIZE;
        size_t col_begin = block_col * BLOCK_SIZE;
        size_t row_end = row_begin + BLOCK_SIZE;
        size_t col_end = row_end + BLOCK_SIZE;
        for (size_t row = row_begin; row < row_end; ++row) {
          for (size_t k = 0; k < size_; ++k) {
            for (size_t col = col_begin; col < col_end; ++col) {
              result(row, col) += left(row, k) * right(k, col);
            }
          }
        }
        
      }
    });
  }
  
  for (auto& thread : threads) {
    thread.join();
  }
}

int main(int argc, char** argv) {
  assert(argc >= 2 && argc <= 4);
  std::string file{argv[1]};
  char mode = (argc >= 3 ? argv[2][0] : '1');
  size_t num_of_threads = (argc >= 4 ? atoi(argv[3]) : 16);
  MatrixMultiplication mul{file, mode, num_of_threads};
  std::cout << mul.Run();
  return 0;
}