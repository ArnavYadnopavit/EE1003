import ctypes
import numpy as np
import matplotlib.pyplot as plt

# Define the C library and load it
lib = ctypes.CDLL('./func.so')  # Ensure the correct path to your compiled C library

ORDER = 2

class Matrix(ctypes.Structure):
    _fields_ = [("mat", ctypes.c_double * ORDER * ORDER)]

# Create the matrix in Python
matrix_data = [[1, -1], [3, -3]]
matrix_instance = Matrix()

for i in range(ORDER):
    for j in range(ORDER):
        matrix_instance.mat[i][j] = matrix_data[i][j]

# Define the is_singular function
lib.is_singular.argtypes = [Matrix]
lib.is_singular.restype = ctypes.c_int

# Check if the matrix is singular
if lib.is_singular(matrix_instance):
    print("The matrix is singular.")
else:
    print("The matrix is not singular.")

# Plot the lines x - y = 8 and 3x - 3y = 16
x = np.linspace(-10, 10, 400)
y1 = x - 8
y2 = x - (16 / 3)

plt.plot(x, y1, label='x - y = 8')
plt.plot(x, y2, label='3x - 3y = 16')
plt.xlabel('x')
plt.ylabel('y')
plt.legend()
plt.grid(True)
plt.axhline(0, color='black',linewidth=0.5)
plt.axvline(0, color='black',linewidth=0.5)
plt.savefig("../figs/fig.png")
plt.show()

