import matplotlib.pyplot as plt
import numpy as np

lines = open("data.txt").readlines()[0:]
data = np.array(list(map(lambda line: np.array(list(map(lambda x: int(x), line.split(' ')[0:4]))), lines)))

threads = range(1, 17)
plt.figure('average')
plt.title('Average operation time')
plt.scatter(threads, data.T[0], color = 'red', label='balanced')
plt.scatter(threads, data.T[2], color = 'blue', label='disbalanced')
plt.xlabel('Number of threads')
plt.ylabel('Average time, nsec')
plt.legend()
plt.savefig('average2.png')

plt.figure('max')
plt.title('Max operation time')
plt.scatter(threads, data.T[1], color = 'red', label='balanced')
plt.scatter(threads, data.T[3], color = 'blue', label='disbalanced')
plt.xlabel('Number of threads')
plt.ylabel('Max time, mcsec')
plt.legend()
plt.savefig('max2.png')
