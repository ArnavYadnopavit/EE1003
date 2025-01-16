import numpy as np
import matplotlib.pyplot as plt
import ctypes
from ctypes import POINTER, c_double

# Define complex type for ctypes
class Complex(ctypes.Structure):
    _fields_ = [("real", ctypes.c_double), ("imag", ctypes.c_double)]

    def __complex__(self):
        return self.real + self.imag * 1j


# Define the matrix struct
class Matrix(ctypes.Structure):
    _fields_ = [("mat", Complex * 2 * 2)]  # Assuming ORDER = 2


# Load the compiled C libraries
# Replace these paths with the actual locations of your compiled shared libraries
eigen_lib = ctypes.CDLL("./func.so")
newton_lib = ctypes.CDLL("./func.so")

# Declare the function signatures
eigen_lib.QRAlgorithm.restype = POINTER(Complex)
eigen_lib.QRAlgorithm.argtypes = [Matrix]
newton_lib.newton.restype = POINTER(c_double)
print("hi")

def extract_eigenvalues():
    """Extract eigenvalues from the hardcoded matrix {{0, 1}, {273, -32}}."""
    # Define the hardcoded matrix
    matrix = Matrix()
    matrix.mat[0][0] = Complex(0, 0)       # mat[0][0] = 0
    matrix.mat[0][1] = Complex(1, 0)       # mat[0][1] = 1
    matrix.mat[1][0] = Complex(273, 0)     # mat[1][0] = 273
    matrix.mat[1][1] = Complex(-32, 0)     # mat[1][1] = -32

    # Call the QRAlgorithm function
    eigenvalues_ptr = eigen_lib.QRAlgorithm(matrix)

    # Retrieve eigenvalues from the pointer
    eigenvalues = [complex(eigenvalues_ptr[i]) for i in range(2)]  # Assuming ORDER = 2

    # Free allocated memory in C
    eigen_lib.free(eigenvalues_ptr)

    return eigenvalues

print("hi")
def extract_roots():
    """Call the newton method from the C library and retrieve roots."""
    # Call the newton function
    roots_ptr = newton_lib.newton()

    # Retrieve the roots (assuming 2 roots)
    roots = [roots_ptr[i] for i in range(2)]

    # Free allocated memory in C
    newton_lib.free(roots_ptr)

    return roots


# Extract eigenvalues and roots
eigenvalues = extract_eigenvalues()
roots = extract_roots()

# Extract real parts for plotting
real_parts = [eig.real for eig in eigenvalues]

# Plot results
plt.figure(figsize=(8, 6))

# Theoretical Plot
X = np.linspace(-100, 100, 2000)
Y = X**2 + 32 * X - 273
plt.plot(X, Y, label="Theoretical Curve", color='red')

# Plot eigenvalues
plt.scatter(real_parts, [0, 0], color='blue', label="Eigenvalues of Companion matrix")

# Plot roots
plt.scatter(roots, [0, 0], color='green', label="Roots")

# Annotate points
for i, root in enumerate(roots):
    plt.annotate(f'Root {i + 1} ({root:.2f})',
                 (root, 0),
                 textcoords="offset points",
                 xytext=(10, 10),
                 ha='center')

for i, eig in enumerate(real_parts):
    plt.annotate(f'Eigen {i + 1} ({eig:.2f})',
                 (eig, 0),
                 textcoords="offset points",
                 xytext=(-10, -15),
                 ha='center')

# Customize the plot
plt.xlabel("x")
plt.ylabel("f(x)")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.savefig("../figs/fig.png")
plt.show()

