#include <assert.h>
#include <chrono>
#include <iostream>
#include <fstream>

#include <stdlib.h>

namespace {
  const size_t BLOCK_SIZE = 16; 
}

class MatrixMultiplication{
  public:
    MatrixMultiplication(const std::string& filename, char mode = '1');
    int Run();
  private:
    const char mode_;
    int* left_;
    int* right_;
    int* result_;
    size_t size_;
    size_t capacity_;

    int& at(int* arr, size_t row, size_t col) { return arr[row * capacity_ + col]; }
    int& left(size_t row, size_t col) { return at(left_, row, col); }
    int& right(size_t row, size_t col) { return at(right_, row, col); }
    int& result(size_t row, size_t col) { return at(result_, row, col); }

    void ReadArrayFrom(std::ifstream& f_in, int*& arr);
    void ReadFile(const std::string& filename);
    void NaiveSingleThread();
    void BlockSingleThread();
};

MatrixMultiplication::MatrixMultiplication(const std::string& filename, char mode)
    : mode_(mode) {
  if (mode < '1' || mode > '4') {
    throw std::invalid_argument("mode should be from 1 to 4");
  }
  ReadFile(filename);
  result_ = reinterpret_cast<int*>(std::malloc(sizeof(int) * capacity_ * size_));
  for (int i = 0; i < capacity_ * size_; ++i) {
    result_[i] = 0;
  }
}

void MatrixMultiplication::ReadArrayFrom(std::ifstream& f_in, int*& arr) {
  arr = reinterpret_cast<int*>(sizeof(int) * capacity_ * size_);
  for (size_t i = 0; i < size_; ++i) {
    for (size_t j = 0; j < size_; ++j)
      f_in >> at(arr, i, j);
  }
}

void MatrixMultiplication::ReadFile(const std::string& filename) {
  std::ifstream f_in(filename);
  if (!f_in.is_open()) {
    throw "failed to open " + filename;
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
// static int counter = 0;
void MatrixMultiplication::BlockSingleThread() {
  const size_t full_blocks = size_ / BLOCK_SIZE; 
  const size_t num_of_blocks = full_blocks + static_cast<bool>(size_ % BLOCK_SIZE);
  for (size_t row_1 = 0; row_1 < size_; row_1 += BLOCK_SIZE) {
    for (size_t col_1 = 0; col_1 < size_; col_1 += BLOCK_SIZE) {
      for (size_t row_2 = 0; row_2 < size_; row_2 += BLOCK_SIZE) {
        for (size_t col_2 = 0; col_2 < size_; col_2 += BLOCK_SIZE) {
          size_t row_max = std::min(row_1 + BLOCK_SIZE, size_);
          size_t col_max = std::min(col_2 + BLOCK_SIZE, size_);
          size_t k_max = std::min(std::min(BLOCK_SIZE, size_ - col_1),
            size_ - row_2);
          for (size_t row = row_1; row < row_max; ++row) {
            for (size_t k = 0; k < k_max; ++k) {
              for (size_t col = col_2; col < col_max; ++col) {
                result(row, col) += left(row, k + col_1) * right(k + row_2, col);
              }
            }
          }
        }
      }
    }
  }
}

int main(int argc, char** argv) {
  assert(argc == 2 || argc == 3);
  MatrixMultiplication mul{std::string(argv[1]), (argc == 3 ? argv[2][0] : '1')};
  std::cout << mul.Run();
  // std::cout << std::endl << counter << std::endl;
  return 0;
}