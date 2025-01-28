import ctypes
import numpy as np
import matplotlib.pyplot as plt

# Load the shared library
func = ctypes.CDLL('./func.so')

# Define the argument and return types for the C function
func.simulate_bernoulli.argtypes = [ctypes.c_double, ctypes.c_int, ctypes.POINTER(ctypes.c_double)]
func.simulate_bernoulli.restype = None

# Parameters for the Bernoulli distribution
p_a = 1 / 12  # Probability of event A (success)
n_simulations = 10000  # Number of simulations

# Allocate memory for the result (empirical PMF)
empirical_pmf = (ctypes.c_double * 2)()

# Call the C function
func.simulate_bernoulli(ctypes.c_double(p_a), ctypes.c_int(n_simulations), empirical_pmf)

# Convert the result from ctypes array to a Python list
empirical_pmf = [empirical_pmf[0], empirical_pmf[1]]

# Theoretical PMF
theoretical_pmf = [1 - p_a, p_a]

# Plot comparison
x = [0, 1]  # Possible outcomes

plt.figure(figsize=(8, 5))
plt.stem(x, theoretical_pmf, linefmt='b-', markerfmt='bo', basefmt=" ", label="theory")
plt.stem(x, empirical_pmf, linefmt='r--', markerfmt='ro', basefmt=" ", label="sim")

# Add text annotations for the probabilities
for i, (xt, yt, ys) in enumerate(zip(x, theoretical_pmf, empirical_pmf)):
    plt.text(xt - 0.03, yt + 0.05, f"{yt:.3f}", color="blue", fontsize=10)  # Theoretical
    plt.text(xt - 0.03, ys + 0.1, f"{ys:.3f}", color="red", fontsize=10)   # Simulated

plt.xlabel("$x$")
plt.ylabel("$p_X(x)$")
plt.xticks([0, 1], labels=["0 (Not A)", "1 (A)"])
plt.ylim(0, 1.2)
plt.legend()
plt.grid(axis="y", linestyle="--", alpha=0.7)
plt.savefig("../figs/fig.png")
plt.show()

