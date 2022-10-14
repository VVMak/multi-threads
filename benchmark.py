import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt
import subprocess
import sys


MIN_SIZE=128
MAX_SIZE=128 * 16 + 1
STEP_SIZE=128

MIN_THREADS = 2
MAX_THREADS = 17
STEP_THREADS = 1
SIZE = int(np.sqrt(5 * (2 ** 20)))

NUM_OF_TESTS = 5

TEMP_TEST_FILE_NAME = 'test.tmp'
BIN_FILENAME = 'matrix_mul'
PICTURE_SERIES = 'graph'

def compile_file(file, outfile):
    result = subprocess.run(["g++", "-std=c++11", "-O4", "-o", outfile, file],\
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if result.returncode != 0:
        raise Exception(f"can't compile {file}")
def compile_input_file():
    if len(sys.argv) == 1:
        raise Exception("There are no programs to test")
    file = sys.argv[1]
    compile_file(file, BIN_FILENAME)
def execute_program(file, mode = '1', thread_num = 16):
    return int(subprocess.check_output([f"./{file}", TEMP_TEST_FILE_NAME, mode, str(thread_num)]))
def remove_file(file):
    Path(file).unlink(missing_ok=True)
def create_file(file):
    Path(file).touch(exist_ok=False)
def generate_matrix(size):
    return np.random.randint(-100, 100, size=(size, size))
def generate_test(size):
    matrices = np.random.randint(-100, 100, size=(size * 2, size))
    np.savetxt(TEMP_TEST_FILE_NAME, matrices, fmt="%d", header=str(size), comments="")

if len(sys.argv) < 2:
    raise Exception("No picture name")
if len(sys.argv) > 2:
    raise Exception("Too many arguments")

compile_input_file()

sizes = np.arange(MIN_SIZE, MAX_SIZE, STEP_SIZE)
times1 = []
times2 = []
for SIZE in sizes:
    print(SIZE)
    average1 = np.array([])
    average2 = np.array([])
    for _ in np.arange(0, NUM_OF_TESTS):
        generate_test(SIZE)
        average1 = np.append(average1, execute_program(BIN_FILENAME, '1'))
        average2 = np.append(average2, execute_program(BIN_FILENAME, '2'))
    times1 = np.append(times1, np.average(average1))
    times2 = np.append(times2, np.average(average2))

plt.figure('single-thread')
plt.title('Однопоточные реализации')
plt.scatter(sizes, times1, color='red', label='наивная')
plt.scatter(sizes, times2, color='blue', label='блочная')
plt.xlabel('Размер матрицы')
plt.ylabel(f'Время (среднее за {NUM_OF_TESTS} попыток), мсек')
plt.legend()
plt.savefig(f'{PICTURE_SERIES}1.png')


threads = np.arange(MIN_THREADS, MAX_THREADS, STEP_THREADS)
times3 = []
times4 = []
for num_of_threads in threads:
    print(num_of_threads)
    average3 = np.array([])
    average4 = np.array([])
    for _ in np.arange(0, NUM_OF_TESTS):
        generate_test(SIZE)
        average3 = np.append(average3, execute_program(BIN_FILENAME, '3', num_of_threads))
        average4 = np.append(average4, execute_program(BIN_FILENAME, '4', num_of_threads))
    times3 = np.append(times3, np.average(average3))
    times4 = np.append(times4, np.average(average4))

plt.figure('multi-thread')
plt.title('Многопоточные реализации')
plt.scatter(threads, times3, color='red', label='наивная')
plt.scatter(threads, times4, color='blue', label='блочная')
plt.xlabel('Количество потоков')
plt.ylabel(f'Время (среднее за {NUM_OF_TESTS} попыток), мсек')
plt.legend()
plt.savefig(f'{PICTURE_SERIES}2.png')

remove_file(BIN_FILENAME)
remove_file(TEMP_TEST_FILE_NAME)
