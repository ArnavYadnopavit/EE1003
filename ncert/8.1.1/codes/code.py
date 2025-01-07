# Import necessary libraries
import numpy as np
import matplotlib.pyplot as plt
from ctypes import CDLL, Structure, POINTER, c_double, byref

# Load the shared library
lib = CDLL("./func.so")

# Specify the argument and return types for the C function
lib.integrated.argtypes = [c_double, c_double]
lib.integrated.restype = c_double

# Define initial conditions
x2 = 4  # value of x2
x1 = 1   # value for x1


plt.figure(figsize=(8, 6))

#Theoritical Plot
X = np.linspace(0, 5, 5000)
Y = np.sqrt(X)
plt.plot(X, Y, label="$y^2=x$", color='red') #plotting y^2=x
plt.axvline(x=1, color='blue', linestyle=':', linewidth=2, label='$x = 1$') #plotting x=1
plt.axvline(x=4, color='green', linestyle=':', linewidth=2, label='$x = 4$') #plotting x=4

# Shade the region between x=1 and x=4 under y=sqrt(x)
x_region=np.linspace(x1,x2,3000)
y_region=np.sqrt(x_region)
plt.fill_between(x_region, y_region, color='orange', alpha=0.3, label='Bounded Region')

# Call integrate
Area = lib.integrated(c_double(x1), c_double(x2))
print("Area is",Area)

plt.xlabel("x")
plt.ylabel("y")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.savefig("../figs/fig.png")
