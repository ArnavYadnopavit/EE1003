import ctypes
import numpy as np
import matplotlib.pyplot as plt

# Load the shared library
lib = ctypes.CDLL('./func.so')  # Compile the C code to a shared library (.so)

# Define the matrix structure in Python
class Matrix(ctypes.Structure):
    _fields_ = [("mat", ctypes.c_double * 4)]  # 2x2 matrix flattened to 1D (ORDER * ORDER)

# Define the vector structure in Python
class Vector(ctypes.Structure):
    _fields_ = [("vec", ctypes.c_double * 2)]  # 2-element vector

# Define the C function prototypes
lib.luDecompose.argtypes = [Matrix, ctypes.POINTER(Matrix), ctypes.POINTER(Matrix)]
lib.luDecompose.restype = None

lib.solve.argtypes = [Matrix, Vector]
lib.solve.restype = Vector

# Input matrix A (2x2) and vector b
A = Matrix((1, 1, 1, -1))  # Coefficient matrix
b = Vector((36, 4))  # Right-hand side vector

# Create empty matrices for L and U
L = Matrix((0,) * 4)
U = Matrix((0,) * 4)

# Call the C function for LU decomposition
lib.luDecompose(A, ctypes.byref(L), ctypes.byref(U))

# Solve the system using the solve function from the C library
solution = lib.solve(A, b)

# Convert the results back to Python-readable format
def matrix_to_numpy(mat):
    return np.array(mat.mat).reshape(2, 2)

def vector_to_numpy(vec):
    return np.array(vec.vec)

L_np = matrix_to_numpy(L)
U_np = matrix_to_numpy(U)
solution_np = vector_to_numpy(solution)

# Print the results
print("Input Matrix A:")
print(matrix_to_numpy(A))
print("\nLower Triangular Matrix L:")
print(L_np)
print("\nUpper Triangular Matrix U:")
print(U_np)
print("\nSolution Vector x:")
print(solution_np)

# Plot the lines x + y = 36 and x - y = 4
x = np.linspace(-20, 60, 400)
y1 = 36 - x
y2 = x - 4

plt.plot(x, y1, label='x + y = 36')
plt.plot(x, y2, label='x - y = 4')
plt.scatter(solution_np[0], solution_np[1], color='red', label=f'Solution ({solution_np[0]:.2f}, {solution_np[1]:.2f})', zorder=5)
plt.annotate(f'({solution_np[0]:.2f}, {solution_np[1]:.2f})',
             (solution_np[0], solution_np[1]),
             textcoords="offset points",
             xytext=(-10, -15),
             ha='center')
plt.xlabel('x')
plt.ylabel('y')
plt.legend()
plt.grid(True)
plt.axhline(0, color='black', linewidth=0.5)
plt.axvline(0, color='black', linewidth=0.5)
plt.axis('equal')
plt.savefig("../figs/fig.png")
plt.show()

