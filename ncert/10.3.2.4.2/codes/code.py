import ctypes
import numpy as np
import matplotlib.pyplot as plt

# Load the shared library
lib = ctypes.CDLL('./func.so')  # Compile the C code to a shared library (.so)

# Define the matrix structure in Python
class Matrix(ctypes.Structure):
    _fields_ = [("mat", ctypes.c_double * 4)]  # 2x2 matrix flattened to 1D (ORDER * ORDER)

# Define the C function prototypes
lib.luDecompose.argtypes = [Matrix, ctypes.POINTER(Matrix), ctypes.POINTER(Matrix)]
lib.luDecompose.restype = None

# Input matrix A (2x2)
A = Matrix((1, -1, 3, -3))

# Create empty matrices for L and U
L = Matrix((0,) * 4)
U = Matrix((0,) * 4)

# Call the C function for LU decomposition
lib.luDecompose(A, ctypes.byref(L), ctypes.byref(U))

# Convert the results back to Python-readable format
def matrix_to_numpy(mat):
    return np.array(mat.mat).reshape(2, 2)

L_np = matrix_to_numpy(L)
U_np = matrix_to_numpy(U)

# Print the results
print("Input Matrix A:")
print(matrix_to_numpy(A))
print("\nLower Triangular Matrix L:")
print(L_np)
print("\nUpper Triangular Matrix U:")
print(U_np)


# Plot the lines x - y = 8 and 3x - 3y = 16
x = np.linspace(-20, 20, 400)
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
plt.axis('equal')
plt.savefig("../figs/fig.png")
plt.show()

