import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt
import subprocess
import sys
from time import sleep


MIN_SIZE=100
MAX_SIZE=150
STEP=10
NUM_OF_TESTS=10

TEMP_TEST_FILE_NAME = 'test.tmp'
BIN_FILENAME = 'matrix_mul'

def compile_file(file, outfile):
    result = subprocess.run(["g++", "-std=c++11", "-o", outfile, file],\
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if result.returncode != 0:
        raise Exception(f"can't compile {file}")
def compile_input_file():
    if len(sys.argv) == 1:
        raise Exception("There are no programs to test")
    if len(sys.argv) > 2:
        raise Exception("Too many arguments")
    file = sys.argv[1]
    compile_file(file, BIN_FILENAME)
def execute_program(file, mode = '1'):
    return int(subprocess.check_output([f"./{file}", TEMP_TEST_FILE_NAME, mode]))
def remove_file(file):
    Path(file).unlink(missing_ok=True)
def create_file(file):
    Path(file).touch(exist_ok=False)
def generate_matrix(size):
    return np.random.randint(-100, 100, size=(size, size))
def generate_test(size):
    matrices = np.random.randint(-100, 100, size=(size * 2, size))
    np.savetxt(TEMP_TEST_FILE_NAME, matrices, fmt="%d", header=str(size), comments="")

sizes = np.arange(MIN_SIZE, MAX_SIZE, STEP)
times1 = []
times2 = []
compile_input_file()
for size in sizes:
    print(size)
    average1 = np.array([])
    average2 = np.array([])
    for _ in np.arange(0, NUM_OF_TESTS):
        generate_test(size)
        average1 = np.append(average1, execute_program(BIN_FILENAME, '1'))
        average2 = np.append(average2, execute_program(BIN_FILENAME, '2'))
    times1 = np.append(times1, np.average(average1))
    times2 = np.append(times2, np.average(average2))
remove_file(BIN_FILENAME)
remove_file(TEMP_TEST_FILE_NAME)

plt.title('Наивная реализация')
plt.plot(sizes, times1, color='red')
plt.plot(sizes, times2, color='blue')
plt.xlabel('Размер матрицы')
plt.ylabel('Время, мсек')
plt.savefig('attempt3.png')