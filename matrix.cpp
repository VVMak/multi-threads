#include <assert.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>

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
    std::vector<std::vector<int>> left_;
    std::vector<std::vector<int>> right_;
    std::vector<std::vector<int>> result_;

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
  const int size = left_.size();
  result_.resize(size);
  for (auto& v : result_) {
    v.resize(size);
  }
}

void MatrixMultiplication::ReadFile(const std::string& filename) {
  std::ifstream f_in(filename);
  if (!f_in.is_open()) {
    throw "failed to open " + filename;
  }
  int size = 0;
  f_in >> size;
  left_.resize(size);
  for (auto& v : left_) {
    v.resize(size);
    for (auto& x : v) {
      f_in >> x;
    }
  }
  right_.resize(size);
  for (auto& v : right_) {
    v.resize(size);
    for (auto& x : v) {
      f_in >> x;
    }
  }
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
  const size_t size = left_.size();
  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < size; ++j) {
      for (size_t k = 0; k < size; ++k) {
        result_[i][j] += left_[i][k] * right_[k][j];
      }
    }
  }
}
// static int counter = 0;
void MatrixMultiplication::BlockSingleThread() {
  const size_t size = left_.size();
  const size_t full_blocks = size / BLOCK_SIZE; 
  const size_t num_of_blocks = full_blocks + static_cast<bool>(size % BLOCK_SIZE);
  for (size_t row_1 = 0; row_1 < size; row_1 += BLOCK_SIZE) {
    for (size_t col_1 = 0; col_1 < size; col_1 += BLOCK_SIZE) {
      for (size_t row_2 = 0; row_2 < size; row_2 += BLOCK_SIZE) {
        for (size_t col_2 = 0; col_2 < size; col_2 += BLOCK_SIZE) {
          size_t row_max = std::min(row_1 + BLOCK_SIZE, size);
          size_t col_max = std::min(col_2 + BLOCK_SIZE, size);
          size_t k_max = std::min(std::min(BLOCK_SIZE, size - col_1),
            size - row_2);
          // std::cout << row_1 << " " << row_max << ' ' << col_2 << ' ' << col_max << ' ' << k_max << std::endl;
          for (size_t row = row_1; row < row_max; ++row) {
            for (size_t k = 0; k < k_max; ++k) {
              for (size_t col = col_2; col < col_max; ++col) {
                result_[row][col] += left_[row][k + col_1] * right_[k + row_2][col];
                  // ++counter;
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