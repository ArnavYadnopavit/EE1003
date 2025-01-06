# Import necessary libraries
import numpy as np
import matplotlib.pyplot as plt
from ctypes import CDLL, Structure, POINTER, c_float, byref


# Define the structure matching the C struct
class Coords(Structure):
    _fields_ = [("x", c_float), ("y", c_float)]

# Load the shared library
lib = CDLL("./func.so")

# Declare the return type and argument types for fx
lib.fx.restype = POINTER(Coords)  # Pointer to Coords array
lib.fx.argtypes = [c_float, c_float]  # float, float

# Define initial conditions
yn = 0.5  # Initial value for yn
x = 0.0   # Initial value for x


plt.figure(figsize=(8, 6))

#Theoritical Plot
X = np.linspace(0, 2, 2000)
Y = (np.exp(X)/2)
plt.plot(X, Y, label="theory", color='red')

# Call fx
results = lib.fx(c_float(yn), c_float(x))
xvals = np.array([results[i].x for i in range(2000)]) #Extracting Values
yvals = np.array([results[i].y for i in range(2000)])
plt.plot(xvals, yvals,linestyle=':',label='sim',color='black')



plt.xlabel("x")
plt.ylabel("y")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.legend()
plt.savefig("../figs/fig.png")
