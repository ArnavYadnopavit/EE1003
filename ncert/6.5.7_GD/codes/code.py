import numpy as np
import matplotlib.pyplot as plt
from ctypes import cdll, c_double, Structure

# Load the shared library
gradient_lib = cdll.LoadLibrary("./func.so")  # Use "./gradient.dll" on Windows

# Define the structures as Python-compatible
class Coords(Structure):
    _fields_ = [("x", c_double), ("y", c_double)]

class Gradient(Structure):
    _fields_ = [("min", Coords), ("max", Coords)]

# Set up the function prototype
gradient_lib.g.argtypes = [c_double, c_double]
gradient_lib.g.restype = Gradient

# Call the function and print results
lower = 0.0  # Define the lower bound
upper = 3.0   # Define the upper bound

# Call the gradient computation function
result = gradient_lib.g(lower, upper)

# Print the global maximum and minimum values
print("Computed Gradient Values:")
print(f"Global Maximum -> x: {result.max.x}, y: {result.max.y}")
print(f"Global Minimum -> x: {result.min.x}, y: {result.min.y}")



plt.figure(figsize=(8, 6))

#Theoritical Plot
X = np.linspace(0, 3, 10000)
Y = 3*X**4-8*X**3+12*X**2-48*X+25;
plt.plot(X, Y, label="theory", color='red')
plt.plot(result.max.x, result.max.y, 'ro', label=f"Global Max: ({result.max.x:.2f}, {result.max.y:.2f})")
plt.plot(result.min.x, result.min.y, 'go', label=f"Global Min: ({result.min.x:.2f}, {result.min.y:.2f})")



plt.xlabel("x")
plt.ylabel("y")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.savefig("../figs/fig.png")
